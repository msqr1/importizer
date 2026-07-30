#include "importizer/Opts.hh"
#include "utils/Fs.hh"
#include "utils/Glob.hh"
#include "utils/Log.hh"
#include <clang/Tooling/JSONCompilationDatabase.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/YAMLTraits.h>
#include <llvm/Support/raw_ostream.h>
#include <memory>
#include <optional>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

namespace cl = llvm::cl;
namespace tl = clang::tooling;
namespace pth = llvm::sys::path;
namespace yml = llvm::yaml;

template <unsigned len>
struct SmallStrParser : public cl::parser<llvm::SmallString<len>> {
  bool parse(cl::Option &, llvm::StringRef, llvm::StringRef val,
             llvm::SmallString<len> &dst) const {
    dst = val;
    return false;
  }
  SmallStrParser(cl::Option &opt) : cl::parser<llvm::SmallString<len>>{opt} {}
};

struct NormalExplicit {
  std::optional<std::vector<llvm::StringRef>> globExprs;
  std::optional<std::vector<llvm::StringRef>> compileFlags;
};

struct NormalOpts {
  llvm::StringRef inDir;
  std::optional<llvm::StringRef> outDir;
  std::optional<llvm::StringRef> dbPath;
  std::optional<NormalExplicit> xplicit;
};

template <> struct yml::MappingTraits<NormalExplicit> {
  static void mapping(yml::IO &in, NormalExplicit &xplicit) {
    in.mapOptional("globs", xplicit.globExprs);
    in.mapOptional("compileFlags", xplicit.compileFlags);
  }
};

template <> struct yml::MappingTraits<NormalOpts> {
  static void mapping(yml::IO &in, NormalOpts &opts) {
    in.mapRequired("inDir", opts.inDir);
    in.mapOptional("outDir", opts.outDir);
    in.mapOptional("compilationDb", opts.dbPath);
    in.mapOptional("explicit", opts.xplicit);
  }
};

void ymlDiagHandler(const llvm::SMDiagnostic &diag, void *) {
  diag.print(g_logOpts->prog.data(), *g_logOpts->target);
}

bool getOpts(const int argc, const char *const *argv, Opts &opts) noexcept {
  // LLVM default options will mix into ours if we don't make our own category
  cl::OptionCategory cat{g_logOpts->prog};
  cl::opt<llvm::SmallString<128>, false, SmallStrParser<128>> config{
      cl::cat(cat),
      cl::desc("<configuration file>"),
      cl::init(llvm::StringRef{"importizer.yml"}),
      cl::Positional,
      cl::ValueOptional,
  };
  cl::opt<llvm::SmallString<128>, true, SmallStrParser<128>> outDir{
      cl::cat(cat),
      "outDir",
      cl::desc(
          "Override the output directory specified in the configuration file"),
      cl::value_desc("directory"),
      cl::location(opts.outDir),
  };
  cl::alias _{"o", cl::aliasopt(outDir)};

  cl::SetVersionPrinter([](llvm::raw_ostream &s) { s << "3.0.0\n"; });
  cl::HideUnrelatedOptions(cat);
  auto &optMap{cl::getRegisteredOptions()};

  // Reset default descriptions to be consistent with the README
  optMap["help"]->setDescription("Display available options");
  optMap["help-list"]->setDescription("Display list of available options");
  optMap["version"]->setDescription("Display version");

  if (!cl::ParseCommandLineOptions(argc, argv,
                                   "importizer - Automagically rewrite "
                                   "header-based C++ into using modules",
                                   g_logOpts->target)) {
    return false;
  }

  const auto buf{llvm::MemoryBuffer::getFile(config, true)};
  if (!buf) {
    return err("Unable to read {}: {}", config, buf.getError().message());
  }
  yml::Input yin{(**buf).getBuffer(), nullptr, ymlDiagHandler};
  NormalOpts nOpts;
  if ((yin >> nOpts).error()) {
    return false;
  }

  // inDir
  opts.inDir = nOpts.inDir;
  llvm::StringRef configDir{pth::parent_path(config)};
  makeRelative(opts.inDir, configDir);

  // outDir
  if (nOpts.outDir && !opts.outDir.empty()) {
    warn("outDir from CLI will override config file");
  } else if (opts.outDir.empty()) {
    if (!nOpts.outDir) {
      return err(
          "outDir must be specified on CLI or in config file as a string");
    }
    opts.outDir = *nOpts.outDir;
  }
  makeRelative(opts.outDir, configDir);

  // compilationDb
  if (nOpts.dbPath) {
    if (nOpts.xplicit) {
      warn("Key 'compilationDb' will take precedence over 'explicit'");
    }
    std::string msg;
    if (!opts.fileHelper.emplace<std::unique_ptr<tl::JSONCompilationDatabase>>(
            tl::JSONCompilationDatabase::loadFromFile(
                *nOpts.dbPath, msg, tl::JSONCommandLineSyntax::AutoDetect))) {
      return err("Unable to parse compilation database: {}", msg);
    }
  }
  // explicit
  else {
    Explicit &xplicit{opts.fileHelper.emplace<Explicit>()};

    // explicit.globs
    std::vector<Glob> globs;
    for (llvm::StringRef globExpr :
         nOpts.xplicit &&nOpts.xplicit->globExprs
             ? *nOpts.xplicit->globExprs
             : std::vector<llvm::StringRef>{"!CMakeLists.txt"}) {
      std::optional<Glob> g{mkGlob(globExpr)};
      if (!g) {
        return false;
      }
      globs.emplace_back(std::move(*g));
    }
    llvm::StringRef path;
    auto checkInDir{[&](const fs::directory_entry &ent) {
      path = ent.path();
      for (const Glob &g : globs) {
        if (g.match(pth::filename(path))) {
          xplicit.files.emplace_back(path);
          break;
        }
      }
      return true;
    }};
    if (!iterateDir(opts.inDir, checkInDir)) {
      return false;
    }

    // explicit.compileFlags
    if (nOpts.xplicit && nOpts.xplicit->compileFlags) {
      for (llvm::StringRef compileFlag : *nOpts.xplicit->compileFlags) {
        xplicit.compileFlags.emplace_back(compileFlag);
      }
    }
  }
  return true;
}
