#pragma once
#include <clang/Tooling/CompilationDatabase.h>
#include <llvm/ADT/SmallString.h>
#include <memory>
#include <string>
#include <vector>

namespace tl = clang::tooling;

struct Opts {
  bool write;
  bool stdImport;
  std::unique_ptr<tl::CompilationDatabase> compDB;
  std::string root;
  std::vector<std::string> hdrs;
  std::vector<std::string> srcs;

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
