#pragma once
#include <clang/Tooling/JSONCompilationDatabase.h>
#include <filesystem>
#include <memory>
#include <optional>
#include <variant>
#include <vector>

namespace fs = std::filesystem;
using namespace clang::tooling;

struct Bootstrap {
  std::vector<fs::path> hdrExts;
  std::vector<fs::path> srcExts;
  std::vector<fs::path> includePaths;
};

struct Opts {
  fs::path inDir;
  fs::path outDir;
  std::variant<std::unique_ptr<JSONCompilationDatabase>, Bootstrap> fileHelper;
};

[[nodiscard]] std::optional<bool> getOpts(const int argc, const char **argv,
                                          Opts &opts) noexcept;
