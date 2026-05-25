#include "utils/FileOp.hh"
#include "utils/Log.hh"
#include <algorithm>
#include <filesystem>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

namespace fs = std::filesystem;
bool cmpDir(const fs::path &dir, const fs::path &refDir) noexcept {
  std::error_code errCode;
  bool res{true};
  fs::recursive_directory_iterator it{refDir, errCode};
  if (errCode) {
    err("Unable to iterate {}: {}\n", refDir, errCode.message());
    return false;
  }
  fs::path path;
  fs::path refPath;
  fs::path relPath;
  std::vector<fs::path> refRelPaths;
  std::string out;
  std::string ref;
  for (const fs::directory_entry &ent : it) {
    if (!ent.is_regular_file()) {
      continue;
    }
    refPath = ent.path();
    relPath = refPath.lexically_relative(refDir);
    path = dir / relPath;
    if (!fs::exists(path)) {
      err("Not found: {}", relPath);
      res = false;
      continue;
    }
    if (!(readToStr(path, out) && readToStr(refPath, ref))) {
      return false;
    }
    if (out != ref) {
      err("Mismatched content for {}\n", relPath);
      res = false;
    }
    refRelPaths.emplace_back(std::move(relPath));
  }
  it = {dir, errCode};
  if (errCode) {
    err("Unable to iterate {}: {}\n", dir, errCode.message());
    return false;
  }
  for (const fs::directory_entry &ent : it) {
    relPath = ent.path().lexically_relative(dir);
    if (std::ranges::find(refRelPaths, relPath) != refRelPaths.end()) {
      err("Unexpected file: {}\n", relPath);
      res = false;
    }
  }
  return res;
}
