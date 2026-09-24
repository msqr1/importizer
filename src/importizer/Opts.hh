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
};

struct Opts {
  bool stdImport;
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
