// "Resolved" options (struct Config) that the user passed in through the CLI and
// configuration file. Resolved means globs becomes paths, compilation
// databases availability are handled, etc.

#pragma once
#include "utils/FS.hh"
#include <clang/Tooling/CompilationDatabase.h>
#include <llvm/ADT/StringSet.h>
#include <llvm/Support/Error.h>
#include <memory>
#include <string>
#include <vector>

namespace tl = clang::tooling;

namespace llvm::vfs {
class FileSystem;
}; // namespace llvm::vfs

namespace vfs = llvm::vfs;

struct CLIConfig {
  std::string configPath;
  std::string rootDir;
  bool write{};

  CLIConfig() noexcept = default;
  CLIConfig(CLIConfig &&) noexcept = default;
  auto operator=(CLIConfig &&) noexcept -> CLIConfig & = default;
  CLIConfig(const CLIConfig &) = delete;
  auto operator=(const CLIConfig &) -> CLIConfig & = delete;
};

// Fully-resolved configuration
struct Config {
  // Root directory
  std::string rootDir;

  // Headers to be modularized (resolved from hdrGlobs)
  llvm::StringSet<> hdrs;

  // Sources to be modularized (resolved from srcGlobs)
  std::vector<std::string> srcs;

  // Explicit and default mode compile commands centralized
  std::unique_ptr<tl::CompilationDatabase> compDB;

  // Convert standard #include to import std or std.compat
  bool stdImport{};

  // Actually rewrite files
  bool write{};

  Config() noexcept = default;
  Config(Config &&) noexcept = default;
  auto operator=(Config &&) noexcept -> Config & = default;
  Config(const Config &) = delete;
  auto operator=(const Config &) -> Config & = delete;
};

[[nodiscard]] auto getCLIConfig(int argc, const char *const *argv,
                                vfs::FileSystem &VFS = getRealFS()) noexcept
    -> llvm::Expected<CLIConfig>;

[[nodiscard]] auto resolveConfig(const CLIConfig &cliCfg,
                                 vfs::FileSystem &VFS = getRealFS()) noexcept
    -> llvm::Expected<Config>;
