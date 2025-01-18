#include <fmt/base.h>
#include <fmt/std.h>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <string>
namespace fs = std::filesystem;

void read(const fs::path& path, std::ifstream& ifs, std::string& str) {
   ifs.open(path);
  if (ifs.fail() || ifs.bad()) {
    fmt::println("CmpDir: Unable open {} for reading", path);
    throw 5;
  }
  size_t fsize{fs::file_size(path)};
    str.resize_and_overwrite(fsize, [&](char* newBuf, size_t _) {
    ifs.read(newBuf, fsize);
    return fsize;
  });
  if(ifs.fail() || ifs.bad()) {
    fmt::println("CmpDir: Unable to read from {}", path);
    throw 5;
  }
  ifs.close();
}

// Compares two directories for structure and file content
// Takes two args, compared and reference.
// Returns 0 for all passed test, 1 for a failed test, and 2 for other errors
int main([[maybe_unused]] int argc, const char* const* argv) {
  int res{0};
  const fs::path compared{argv[1]};
  const fs::path reference{argv[2]};
  std::ifstream ifs;
  std::string cmpContent;
  std::string refContent;
  fs::path refPath;
  fs::path relPath;
  fs::path cmpPath;
  for(const auto& ent : fs::recursive_directory_iterator(reference)) {
    if(!ent.is_regular_file()) continue;
    refPath = ent.path();
    relPath = refPath.lexically_relative(reference);
    cmpPath = compared / relPath;
    if(!fs::exists(cmpPath)) {
      fmt::println("File or directory doesn't exist: {}", relPath);
      res = 1;
      continue;
    }
    try {
      read(cmpPath, ifs, cmpContent);
    }
    catch(...) {
      return 2;
    }
    try {
      read(refPath, ifs, refContent);
    }
    catch(...) {
      return 2;
    }
    if(cmpContent == refContent) continue;
    fmt::println("CmpDir: Mismatched content for: {}. Got:\n{}", relPath, cmpContent);
    res = 1;
  }
  return res;
}