#include <fmt/base.h>
#include <fmt/std.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <version>

namespace fs = std::filesystem;
namespace {

void read(const fs::path& path, std::ifstream& ifs, std::string& str) {
  ifs.open(path, std::fstream::binary);
  if(!ifs) {
    fmt::println("CmpDir: Unable open {} for reading", path);
    throw 5;
  }
  uintmax_t fsize{ fs::file_size(path) };
#ifdef __cpp_lib_string_resize_and_overwrite
  str.resize_and_overwrite(fsize, [fsize]
    ([[maybe_unused]] char* _, [[maybe_unused]] uintmax_t _1) {
    return fsize;
  });
#else
  str.resize(fsize);
#endif
  ifs.read(str.data(), fsize);
  if(!ifs) {
    fmt::println("CmpDir: Unable to read from {}", path);
    throw 5;
  }
#ifdef WIN32
  std::erase(str, '\r');
#endif
  ifs.close();
}

}

// Compares two directories for structure and file content
// Takes two args, compared and reference.
// Returns 0 for all passed test, 1 for a failed test, and 2 for other errors
int main([[maybe_unused]] int argc, const char* const* argv) {
  int res{};
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
      fmt::println("CmpDir: File or directory doesn't exist: {}", relPath);
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
    if(cmpContent != refContent) {
      fmt::println("CmpDir: Mismatched content for: {}. Got:\n{}", relPath, cmpContent);
      res = 1;
    }
  }
  return res;
}