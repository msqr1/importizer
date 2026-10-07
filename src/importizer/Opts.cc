#include "importizer/Opts.hh"
#include "utils/Fs.hh"
#include "utils/Glob.hh"
#include "utils/Log.hh"
#include <array>
#include <cassert>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/JSONCompilationDatabase.h>
#include <cstddef>
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/SmallVector.h>
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
#include <span>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

namespace cl = llvm::cl;
namespace tl = clang::tooling;
namespace pth = llvm::sys::path;
namespace yml = llvm::yaml;

namespace {

struct NormalExplicit {
  std::vector<llvm::StringRef> srcGlobExprs;
  std::vector<std::string> compileFlags;
};

struct NormalOpts {
  bool stdImport;
  llvm::StringRef root;
  std::optional<llvm::StringRef> dbPath;
  std::vector<llvm::StringRef> hdrGlobExprs;
  std::optional<NormalExplicit> xplicit;
};

void ymlDiagHandler(const llvm::SMDiagnostic &diag, void *) {
  diag.print(g_logOpts->prog.data(), *g_logOpts->target);
}

bool globDir(llvm::StringRef dir, std::span<std::vector<Glob>> ins,
             std::span<std::vector<std::string> *> outs) {
  assert(ins.size() == outs.size());
  auto checkDir{[&](const fs::directory_entry &ent) {
    llvm::StringRef relPath{ent.path()};
    relPath.consume_front(dir);
    for (size_t i{}; i < ins.size(); ++i) {
      for (const Glob &glob : ins[i]) {
        if (glob.match(ent.path())) {
          outs[i]->emplace_back(ent.path());
        }
      }
    }
    return true;
  }};
  return iterateDir<true>(dir, checkDir);
}

} // namespace

template <> struct yml::MappingTraits<NormalExplicit> {
  static void mapping(yml::IO &in, NormalExplicit &xplicit) {
    in.mapRequired("srcGlobs", xplicit.srcGlobExprs);
    in.mapOptional("compileFlags", xplicit.compileFlags);
  }
};

template <> struct yml::MappingTraits<NormalOpts> {
  static void mapping(yml::IO &in, NormalOpts &opts) {
    in.mapRequired("root", opts.root);
    in.mapRequired("hdrGlobs", opts.hdrGlobExprs);
    in.mapOptional("compilationDb", opts.dbPath);
    in.mapOptional("stdImport", opts.stdImport);
    in.mapOptional("explicit", opts.xplicit);
  }
};

bool getOpts(const int argc, const char *const *argv, Opts &opts) noexcept {
  // LLVM default options will mix into ours if we don't make our own category
  cl::OptionCategory cat{g_logOpts->prog};
  cl::opt<std::string> config{
      cl::cat(cat),
      cl::desc("<YAML configuration file>"),
      cl::init("importizer.yml"),
      cl::Positional,
      cl::ValueOptional,
  };
  cl::opt<bool, true> write{
      cl::cat(cat),
      "write",
      cl::desc("Actually rewrite files. Without this flag it's a dry-run."),
      cl::location(opts.write),
  };
  cl::alias _{"w", cl::aliasopt(write)};

  cl::SetVersionPrinter([](llvm::raw_ostream &s) { s << "3.0.0\n"; });
  cl::HideUnrelatedOptions(cat);
  llvm::DenseMap<llvm::StringRef, cl::Option *> &optMap{
      cl::getRegisteredOptions()};

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

  // root
  opts.root = nOpts.root;
  llvm::StringRef configDir{pth::parent_path(config)};
  mkRelative(opts.root, configDir);
  std::vector<std::vector<Glob>> rootGlobIns;
  std::vector<std::vector<std::string> *> rootGlobOuts;

  // stdImport
  opts.stdImport = nOpts.stdImport;

  // hdrGlobs
  std::vector<Glob> tmp;
  if (!mkGlobs(tmp, nOpts.hdrGlobExprs)) {
    return false;
  }
  rootGlobIns.emplace_back(std::move(tmp));
  rootGlobOuts.emplace_back(&opts.hdrs);
  tmp.clear();

  if (nOpts.dbPath && nOpts.xplicit) {
    return err("'compilationDb' and 'explicit' are mutually exclusive");
  }

  // compilationDb
  else if (nOpts.dbPath) {
    std::string msg;
    std::unique_ptr<tl::JSONCompilationDatabase> jsonCompDb{
        tl::JSONCompilationDatabase::loadFromFile(
            *nOpts.dbPath, msg, tl::JSONCommandLineSyntax::AutoDetect)};
    if (!jsonCompDb) {
      return err("Unable to parse compilation database: {}", msg);
    }
    opts.srcs = jsonCompDb->getAllFiles();
    opts.compDB = std::move(jsonCompDb);
  }

  // explicit
  else if (nOpts.xplicit) {
    // explicit.srcGlobs
    if (!mkGlobs(tmp, nOpts.xplicit->srcGlobExprs)) {
      return false;
    }
    rootGlobIns.emplace_back(std::move(tmp));
    rootGlobOuts.emplace_back(&opts.srcs);
    tmp.clear();

    // explicit.compileFlags
    opts.compDB = std::make_unique<tl::FixedCompilationDatabase>(
        ".", nOpts.xplicit->compileFlags);
  } else {
    return err(
        "At least one of 'compilationDb' or 'explicit' must be specified");
  }
  return globDir(opts.root, rootGlobIns, rootGlobOuts);
}
