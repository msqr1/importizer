#include "importizer/Config.hh"
#include "utils/Diag.hh"
#include "utils/FS.hh"
#include "utils/Glob.hh"
#include <cassert>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/JSONCompilationDatabase.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/StringSet.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/ErrorOr.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/FormatVariadic.h>
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
namespace fs = llvm::sys::fs;
namespace vfs = llvm::vfs;
namespace pth = llvm::sys::path;
namespace yml = llvm::yaml;

void ymlDiagHandler(const llvm::SMDiagnostic &diag, void * /*ctx*/) noexcept {
  assert(globalLogOpts.stream != nullptr && globalLogOpts.prog != nullptr);
  diag.print(globalLogOpts.prog, *globalLogOpts.stream);
}

auto getCLIConfig(const int argc, const char *const *argv, vfs::FileSystem &VFS) noexcept
    -> llvm::Expected<CLIConfig> {
  CLIConfig cliCfg;

  // LLVM default options will mix into ours if we don't make our own category
  cl::OptionCategory cat{globalLogOpts.prog};

  cl::opt<std::string, true> configPath{
      cl::cat{cat},
      cl::desc{"<YAML configuration file>"},
      cl::location(cliCfg.configPath),
      cl::init("importizer.yml"),
      cl::Positional,
      cl::ValueOptional,
  };
  cl::opt<bool, true> write{
      cl::cat{cat},
      "write",
      cl::desc{"Actually rewrite files. Without this flag it's a dry-run."},
      cl::location(cliCfg.write),
      cl::init(false),
  };
  cl::alias writeAlias{"w", cl::aliasopt{write}};
  cl::opt<std::string, true> rootDir{
      cl::cat{cat},
      "root-dir",
      cl::desc{"Override the 'rootDir' key in the configuration file."},
      cl::location(cliCfg.rootDir),
      cl::ValueOptional,
  };
  cl::alias rootDirAlias{"r", cl::aliasopt{rootDir}};

  cl::SetVersionPrinter([](llvm::raw_ostream &stream) -> void { stream << "3.0.0\n"; });
  cl::HideUnrelatedOptions(cat);
  llvm::DenseMap<llvm::StringRef, cl::Option *> &optMap{cl::getRegisteredOptions()};

  // Reset default descriptions to be consistent with the README
  optMap["help"]->setDescription("Display available options.");
  optMap["help-list"]->setDescription("List available options.");
  optMap["version"]->setDescription("Display version.");

  if (!cl::ParseCommandLineOptions(argc, argv,
                                   "importizer - Automagically rewrite "
                                   "header-based C++ into using modules.",
                                   globalLogOpts.stream, &VFS)) {
    return llvm::make_error<EmptyErr>();
  }

  return cliCfg;
}

auto resolveConfig(const CLIConfig &cliCfg, vfs::FileSystem &VFS) noexcept
    -> llvm::Expected<Config> {

  auto buf{VFS.getBufferForFile(cliCfg.configPath, -1, false, false, false)};
  if (std::error_code err{buf.getError()}) {
    return llvm::createStringError(
        err, llvm::formatv("When opening {}: {}.", cliCfg.configPath, err.message()));
  }
  // NOLINTBEGIN
  yml::Input ymlIn{(**buf).getMemBufferRef(),
                   const_cast<void *>(static_cast<const void *>(&cliCfg)), ymlDiagHandler};
  // NOLINTEND
  YMLConfig ymlCfg;
  ymlIn >> ymlCfg;
  if (std::error_code err{ymlIn.error()}) {
    return llvm::make_error<EmptyErr>();
  }

  Config cfg;

  // root
  llvm::SmallString<64> tmp;
  if (!cliCfg.rootDir.empty()) {
    cfg.rootDir = cliCfg.rootDir;
  } else {
    tmp = ymlCfg.rootDir;
    pth::make_absolute(pth::parent_path(cliCfg.configPath), tmp);
    cfg.rootDir = tmp.str();
    tmp.clear();
  }
  if (std::error_code err{VFS.getRealPath(cfg.rootDir, tmp)}) {
    return llvm::createStringError(
        err, llvm::formatv("When resolving root directory {}: {}.", cfg.rootDir, err.message()));
  }
  cfg.rootDir = tmp.str();

  llvm::SmallVector<GlobDirRule, 2> rootGlobRules;

  // hdrGlobs
  rootGlobRules.emplace_back();
  llvm::Expected<GlobStorage> globs{mkGlobs(ymlCfg.hdrGlobExprs)};
  if (!globs) {
    return globs.takeError();
  }
  rootGlobRules[0].globs = std::move(*globs);
  rootGlobRules[0].callback = [&](const vfs::directory_entry &ent) -> void {
    if (ent.type() == fs::file_type::regular_file) {
      cfg.hdrs.insert(ent.path());
    }
  };

  // compilationDB
  if (ymlCfg.compDBPath) {
    buf = VFS.getBufferForFile(*ymlCfg.compDBPath, -1, false, false);
    if (std::error_code err{buf.getError()}) {
      return llvm::createStringError(
          err, llvm::formatv("When opening {}: {}.", *ymlCfg.compDBPath, err.message()));
    }
    std::string msg;
    std::unique_ptr<tl::JSONCompilationDatabase> jsonCompDb{
        tl::JSONCompilationDatabase::loadFromBuffer((**buf).getBuffer(), msg,
                                                    tl::JSONCommandLineSyntax::AutoDetect),
    };
    if (!jsonCompDb) {
      return llvm::createStringError("When parsing JSON compilation database: %s.", msg.c_str());
    }
    cfg.srcs = jsonCompDb->getAllFiles();
    cfg.compDB = std::move(jsonCompDb);
  }

  // explicit
  else if (ymlCfg.xplicit) {
    // explicit.srcGlobs
    rootGlobRules.emplace_back();
    globs = mkGlobs(ymlCfg.xplicit->srcGlobExprs);
    if (!globs) {
      return globs.takeError();
    }
    rootGlobRules[1].globs = std::move(*globs);
    rootGlobRules[1].callback = [&](const vfs::directory_entry &ent) -> void {
      if (ent.type() == fs::file_type::regular_file) {
        cfg.srcs.emplace_back(ent.path());
      }
    };

    // explicit.compileFlags
    cfg.compDB = std::make_unique<tl::FixedCompilationDatabase>(".", ymlCfg.xplicit->compileFlags);
  }
  if (llvm::Error err{globDir(cfg.rootDir, rootGlobRules, VFS)}) {
    return err;
  }

  // stdImport
  cfg.stdImport = ymlCfg.stdImport;

  // write
  cfg.write = cliCfg.write;

  return cfg;
}
