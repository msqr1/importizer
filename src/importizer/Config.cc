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

namespace {

struct NormalExplicit {
  std::vector<llvm::StringRef> srcGlobExprs;
  std::vector<std::string> compileFlags;
};

struct NormalConfig {
  std::optional<NormalExplicit> xplicit;
  std::optional<llvm::StringRef> compDBPath;
  std::vector<llvm::StringRef> hdrGlobExprs;
  llvm::StringRef root;
  bool stdImport{};
};

void ymlDiagHandler(const llvm::SMDiagnostic &diag, void * /*unused*/) noexcept {
  assert(globalLogOpts.stream != nullptr && globalLogOpts.prog != nullptr);
  diag.print(globalLogOpts.prog, *globalLogOpts.stream);
}

} // namespace

template <> struct yml::MappingTraits<NormalExplicit> {
  static void mapping(yml::IO &in, NormalExplicit &normalXplicit) {
    in.mapRequired("srcGlobs", normalXplicit.srcGlobExprs);
    in.mapOptional("compileFlags", normalXplicit.compileFlags);
  }
};

template <> struct yml::MappingTraits<NormalConfig> {
  static void mapping(yml::IO &in, NormalConfig &normalCfg) {
    in.mapRequired("root", normalCfg.root);
    in.mapRequired("hdrGlobs", normalCfg.hdrGlobExprs);
    in.mapOptional("compilationDB", normalCfg.compDBPath);
    in.mapOptional("stdImport", normalCfg.stdImport);
    in.mapOptional("explicit", normalCfg.xplicit);
  }
};

auto resolveConfig(const int argc, const char *const *argv, vfs::FileSystem &VFS) noexcept
    -> llvm::Expected<Config> {
  Config cfg;

  // LLVM default options will mix into ours if we don't make our own category
  cl::OptionCategory cat{globalLogOpts.prog};

  cl::opt<std::string> configPath{
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
      cl::location(cfg.write),
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
    return llvm::make_error<EmptyErr>();
  }

  auto buf{VFS.getBufferForFile(configPath, -1, false, false, false)};
  if (std::error_code errCode{buf.getError()}) {
    return llvm::createStringError(
        errCode, llvm::formatv("When opening {}: {}.", configPath, errCode.message()));
  }
  yml::Input ymlIn{(**buf).getBuffer(), nullptr, ymlDiagHandler};
  NormalConfig nOpts;
  ymlIn >> nOpts;
  if (ymlIn.error()) {
    return llvm::make_error<EmptyErr>();
  }

  // root
  llvm::SmallString<128> tmp{nOpts.root};
  llvm::StringRef configDir{pth::parent_path(configPath)};
  pth::make_absolute(configDir, tmp);
  cfg.root = tmp.str();
  if (std::error_code errCode{VFS.getRealPath(cfg.root, tmp)}) {
    return llvm::createStringError(errCode, "When resolving root directory: %s.",
                                   errCode.message().c_str());
  }
  cfg.root = tmp.str();
  llvm::SmallVector<GlobDirRule, 2> rootGlobRules;

  // stdImport
  cfg.stdImport = nOpts.stdImport;

  // hdrGlobs
  rootGlobRules.emplace_back();
  llvm::Expected<GlobStorage> globs{mkGlobs(nOpts.hdrGlobExprs)};
  if (!globs) {
    return globs.takeError();
  }
  rootGlobRules[0].globs = std::move(*globs);
  rootGlobRules[0].callback = [&](const vfs::directory_entry &ent) -> void {
    if (ent.type() == fs::file_type::regular_file) {
      cfg.hdrs.insert(ent.path());
    }
  };

  if (nOpts.compDBPath && nOpts.xplicit) {

    return llvm::createStringError(
        "Invalid configuration: 'compilationDB' and 'explicit' are mutually exclusive");
  }

  // compilationDB
  if (nOpts.compDBPath) {
    buf = VFS.getBufferForFile(*nOpts.compDBPath, -1, false, false);
    if (std::error_code errCode{buf.getError()}) {
      return llvm::createStringError(
          errCode, llvm::formatv("When opening {}: {}.", configPath, errCode.message()));
    }
    std::string msg;
    std::unique_ptr<tl::JSONCompilationDatabase> jsonCompDb{
        tl::JSONCompilationDatabase::loadFromBuffer((**buf).getBuffer(), msg,
                                                    tl::JSONCommandLineSyntax::AutoDetect),
    };
    if (!jsonCompDb) {
      return llvm::createStringError("When parsing JSON compilation database: %s", msg.c_str());
    }
    cfg.srcs = jsonCompDb->getAllFiles();
    cfg.compDB = std::move(jsonCompDb);
  }

  // explicit
  else if (nOpts.xplicit) {
    // explicit.srcGlobs
    rootGlobRules.emplace_back();
    globs = mkGlobs(nOpts.xplicit->srcGlobExprs);
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
    cfg.compDB = std::make_unique<tl::FixedCompilationDatabase>(".", nOpts.xplicit->compileFlags);
  } else {
    return llvm::createStringError(
        "Invalid configuration: At least one of 'compilationDB' or 'explicit' must be specified");
  }
  if (llvm::Error err{globDir(cfg.root, rootGlobRules, VFS)}) {
    return err;
  }
  return cfg;
}
