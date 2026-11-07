// "Resolved" options (struct Config) that the user passed in through the CLI and
// configuration file. Resolved means globs becomes paths, compilation
// databases availability are handled, etc.

#pragma once
#include "utils/FS.hh"
#include <cassert>
#include <clang/Tooling/CompilationDatabase.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/StringSet.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/YAMLTraits.h>
#include <llvm/Support/raw_ostream.h>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#ifdef _MSC_VER
#define NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#else
#define NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif

struct MoveOnlyToken {
  MoveOnlyToken() noexcept = default;
  MoveOnlyToken(MoveOnlyToken &&) noexcept = default;
  MoveOnlyToken(const MoveOnlyToken &) = delete;
  auto operator=(const MoveOnlyToken &) -> MoveOnlyToken & = delete;
};

#define ATTR_MOVE_ONLY NO_UNIQUE_ADDRESS MoveOnlyToken _moveOnly_

namespace tl = clang::tooling;
namespace yml = llvm::yaml;

namespace llvm::vfs {
class FileSystem;
}; // namespace llvm::vfs

namespace vfs = llvm::vfs;

struct CLIConfig {
  ATTR_MOVE_ONLY;
  std::string configPath;
  std::string rootDir;
  bool write{};
};

struct YMLExplicit {
  std::vector<llvm::StringRef> srcGlobExprs;
  std::vector<std::string> compileFlags;
};

template <> struct yml::MappingTraits<YMLExplicit> {
  static void mapping(yml::IO &in, YMLExplicit &ymlExplicit) noexcept {
    in.mapRequired("srcGlobs", ymlExplicit.srcGlobExprs);
    in.mapOptional("compileFlags", ymlExplicit.compileFlags);
  }
};

struct YMLConfig {
  std::optional<YMLExplicit> xplicit;
  std::optional<llvm::StringRef> compDBPath;
  std::vector<llvm::StringRef> hdrGlobExprs;
  llvm::StringRef rootDir;
  bool stdImport{};
};

template <> struct yml::MappingTraits<YMLConfig> {
  static void mapping(yml::IO &in, YMLConfig &ymlCfg) noexcept {
    assert(in.getContext() != nullptr);
    const CLIConfig &cliCfg{*static_cast<CLIConfig *>(in.getContext())};
    if (cliCfg.rootDir.empty()) {
      in.mapRequired("rootDir", ymlCfg.rootDir);
    } else {
      in.mapOptional("rootDir", ymlCfg.rootDir);
    }
    in.mapRequired("hdrGlobs", ymlCfg.hdrGlobExprs);
    in.mapOptional("compilationDB", ymlCfg.compDBPath);
    in.mapOptional("stdImport", ymlCfg.stdImport, false);
    in.mapOptional("explicit", ymlCfg.xplicit);
  }

  static auto validate(yml::IO & /*ymlIO*/, YMLConfig &ymlCfg) noexcept -> std::string {
    if (ymlCfg.compDBPath && ymlCfg.xplicit) {
      return "'compilationDB' and 'explicit' are mutually exclusive.";
    }
    if (!(ymlCfg.compDBPath || ymlCfg.xplicit)) {
      return "At least one of 'compilationDB' or 'explicit' must be specified.";
    }
    return {};
  }
};

// Fully-resolved configuration
struct Config {
  ATTR_MOVE_ONLY;

  // Root directory
  std::string rootDir;

  // Headers to be modularized (resolved from hdrGlobs)
  llvm::StringSet<> hdrs;

  // Sources to be modularized (resolved from srcGlobs or JSON compilation database)
  std::vector<std::string> srcs;

  // Explicit and default mode compile commands centralized
  std::unique_ptr<tl::CompilationDatabase> compDB;

  // Convert standard #include to import std or std.compat
  bool stdImport{};

  // Actually rewrite files
  bool write{};
};

void ymlDiagHandler(const llvm::SMDiagnostic &diag, void * /*ctx*/) noexcept;

auto getCLIConfig(int argc, const char *const *argv, vfs::FileSystem &VFS = getRealFS()) noexcept
    -> llvm::Expected<CLIConfig>;

auto resolveConfig(const CLIConfig &cliCfg, vfs::FileSystem &VFS = getRealFS()) noexcept
    -> llvm::Expected<Config>;
