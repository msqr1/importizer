// "Resolved" options (struct Opts) that the user passed in through the CLI and
// configuration file. Resolved means globs becomes paths, compilation
// databases availability are handled, etc.

#pragma once
#include <clang/Tooling/CompilationDatabase.h>
#include <llvm/ADT/StringSet.h>
#include <llvm/Support/VirtualFileSystem.h>
#include <memory>
#include <string>
#include <vector>

namespace tl = clang::tooling;
namespace vfs = llvm::vfs;

// Resolved options
struct Opts {
  // Root directory
  std::string root;

  // Headers to be modularized (resolved from hdrGlobs)
  llvm::StringSet<> hdrs;

  // Sources to be modularized (resolved from srcGlobs)
  std::vector<std::string> srcs;

  // Explicit and default mode compile commands centralized
  std::unique_ptr<tl::CompilationDatabase> compDB;

  // Actually rewrite files
  bool write{};

  // Convert standard #include to import std or std.compat
  bool stdImport{};

  // Allow default construction
  Opts() noexcept = default;
  Opts(Opts &&) noexcept = default;
  auto operator=(Opts &&) noexcept -> Opts & = default;
  Opts(const Opts &) = delete;
  auto operator=(const Opts &) -> Opts & = delete;
};

// Populate a struct Opts from CLI args
[[nodiscard]] auto getOpts(int argc, const char *const *argv, Opts &opts,
                           vfs::FileSystem &VFS = *vfs::createPhysicalFileSystem()) noexcept
    -> bool;
