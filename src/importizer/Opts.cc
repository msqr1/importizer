#include "importizer/Opts.hh"
#include "utils/Diag.hh"
#include "utils/FS.hh"
#include "utils/Glob.hh"
#include <array>
#include <cassert>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/JSONCompilationDatabase.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/StringSet.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/VirtualFileSystem.h>
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
namespace vfs = llvm::vfs;
namespace pth = llvm::sys::path;
namespace yml = llvm::yaml;

namespace {

struct NormalExplicit {
  std::vector<llvm::StringRef> srcGlobExprs;
  std::vector<std::string> compileFlags;
};

struct NormalOpts {
  std::optional<NormalExplicit> xplicit;
  std::optional<llvm::StringRef> dbPath;
  std::vector<llvm::StringRef> hdrGlobExprs;
  llvm::StringRef root;
  bool stdImport{};
};

void ymlDiagHandler(const llvm::SMDiagnostic &diag, void * /*unused*/) noexcept {
  diag.print(globalLogOpts.prog, *globalLogOpts.stream);
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
    in.mapOptional("compilationDB", opts.dbPath);
    in.mapOptional("stdImport", opts.stdImport);
    in.mapOptional("explicit", opts.xplicit);
  }
};

auto getOpts(const int argc, const char *const *argv, Opts &opts, vfs::FileSystem &VFS) noexcept
    -> bool {
  // LLVM default options will mix into ours if we don't make our own category
  cl::OptionCategory cat{globalLogOpts.prog};
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
      cl::init(false),
  };
  cl::alias _{"w", cl::aliasopt(write)};

  cl::SetVersionPrinter([](llvm::raw_ostream &stream) -> void { stream << "3.0.0\n"; });
  cl::HideUnrelatedOptions(cat);
  llvm::DenseMap<llvm::StringRef, cl::Option *> &optMap{cl::getRegisteredOptions()};

  // Reset default descriptions to be consistent with the README
  optMap["help"]->setDescription("Display available options.");
  optMap["help-list"]->setDescription("Display list of available options.");
  optMap["version"]->setDescription("Display version.");

  if (!cl::ParseCommandLineOptions(argc, argv,
                                   "importizer - Automagically rewrite "
                                   "header-based C++ into using modules",
                                   globalLogOpts.stream)) {
    return false;
  }

  auto buf{VFS.getBufferForFile(config, 0, false, false)};
  if (!buf) {
    return err("Error opening {}: {}", config, buf.getError().message());
  }
  yml::Input ymlIn{(**buf).getBuffer(), nullptr, ymlDiagHandler};
  NormalOpts nOpts;
  if ((ymlIn >> nOpts).error()) {
    return false;
  }

  // root
  llvm::SmallString<128> tmp{nOpts.root};
  llvm::StringRef configDir{pth::parent_path(config)};
  pth::make_absolute(configDir, tmp);
  opts.root = tmp.str();
  if (std::error_code errCode{VFS.getRealPath(opts.root, tmp)}) {
    return err("Unable to get real path of {}: {}", opts.root, errCode.message());
  }
  opts.root = tmp.str();
  std::array<GlobDirRule, 2> rootGlobRules;

  // stdImport
  opts.stdImport = nOpts.stdImport;

  // hdrGlobs
  if (!mkGlobs(rootGlobRules[0].globs, nOpts.hdrGlobExprs)) {
    return false;
  }

  rootGlobRules[0].callback = [&opts](llvm::StringRef path) -> void {
    opts.hdrs.try_emplace(path);
  };

  if (nOpts.dbPath && nOpts.xplicit) {
    return err("'compilationDB' and 'explicit' are mutually exclusive");
  }

  // compilationDB
  if (nOpts.dbPath) {
    buf = VFS.getBufferForFile(*nOpts.dbPath, 0, false, false);
    if (!buf) {
      return err("Error opening {}: {}", config, buf.getError().message());
    }
    std::string msg;
    std::unique_ptr<tl::JSONCompilationDatabase> jsonCompDb{
        tl::JSONCompilationDatabase::loadFromBuffer((**buf).getBuffer(), msg,
                                                    tl::JSONCommandLineSyntax::AutoDetect),
    };
    if (!jsonCompDb) {
      return err("Error parsing compilation database: {}", msg);
    }
    opts.srcs = jsonCompDb->getAllFiles();
    opts.compDB = std::move(jsonCompDb);
  }

  // explicit
  else if (nOpts.xplicit) {
    // explicit.srcGlobs
    if (!mkGlobs(rootGlobRules[1].globs, nOpts.xplicit->srcGlobExprs)) {
      return false;
    }
    rootGlobRules[1].callback = [&opts](llvm::StringRef path) -> void {
      opts.srcs.emplace_back(path);
    };

    // explicit.compileFlags
    opts.compDB = std::make_unique<tl::FixedCompilationDatabase>(".", nOpts.xplicit->compileFlags);
  } else {
    return err("At least one of 'compilationDB' or 'explicit' must be specified");
  }
  return globDir(opts.root, rootGlobRules, VFS);
}
