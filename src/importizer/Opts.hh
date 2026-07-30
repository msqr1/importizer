#pragma once
#include <clang/Tooling/JSONCompilationDatabase.h>
#include <llvm/ADT/SmallString.h>
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace tl = clang::tooling;

struct Explicit {
  std::vector<llvm::SmallString<128>> files;
  std::vector<std::string> compileFlags;
  // Allow default construction
  Explicit() noexcept = default;

  // Allow moving
  Explicit(Explicit &&) noexcept = default;
  Explicit &operator=(Explicit &&) noexcept = default;

  // Disallow copying
  Explicit(const Explicit &) = delete;
  Explicit &operator=(const Explicit &) = delete;
};

struct Opts {
  llvm::SmallString<128> inDir;
  llvm::SmallString<128> outDir;
  std::variant<std::unique_ptr<tl::JSONCompilationDatabase>, Explicit>
      fileHelper;

  // Allow default construction
  Opts() noexcept = default;

  // Allow moving
  Opts(Opts &&) noexcept = default;
  Opts &operator=(Opts &&) noexcept = default;

  // Disallow copying
  Opts(const Opts &) = delete;
  Opts &operator=(const Opts &) = delete;
};

[[nodiscard]] bool getOpts(const int argc, const char *const *argv,
                           Opts &opts) noexcept;
