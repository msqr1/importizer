#pragma once
#include <clang/Tooling/JSONCompilationDatabase.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/Support/FileSystem.h>
#include <memory>
#include <variant>
#include <vector>

namespace tl = clang::tooling;

struct Bootstrap {
  std::vector<llvm::SmallString<64>> ignores;
  std::vector<llvm::SmallString<128>> includePaths;
};

struct Opts {
  llvm::SmallString<128> inDir;
  llvm::SmallString<128> outDir;
  std::variant<std::unique_ptr<tl::JSONCompilationDatabase>, Bootstrap>
      fileHelper;
};

[[nodiscard]] bool getOpts(const int argc, const char *const *argv,
                           Opts &opts) noexcept;
