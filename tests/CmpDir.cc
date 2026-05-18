#include <cstdint>
#include <filesystem>
#include <fmt/base.h>
#include <fmt/std.h>
#include <fstream>
#include <string>
#include <version>

namespace {
namespace fs = std::filesystem;
bool read(const fs::path &path, std::ifstream &ifs, std::string &str) {
  ifs.open(path, std::fstream::binary);
  if (!ifs) {
    fmt::println("Unable open {} for reading", path);
    return false;
  }
  uintmax_t fsize{fs::file_size(path)};
#ifdef __cpp_lib_string_resize_and_overwrite
  str.resize_and_overwrite(
      fsize, [fsize]([[maybe_unused]] char *_, [[maybe_unused]] uintmax_t _1) {
        return fsize;
      });
#else
  str.resize(fsize);
#endif
  ifs.read(str.data(), fsize);
  if (!ifs) {
    fmt::println("Unable to read from {}", path);
    return false;
  }
#ifdef WIN32
  std::erase(str, '\r');
#endif
  ifs.close();
  return true;
}

} // namespace

// Compares two directories for structure and file content
// Takes two args, compared and reference.
// Returns 0 for all passed test, 1 for a failed test, and 2 for other errors
int main([[maybe_unused]] int argc, const char *const *argv) {
  int res{};
  const fs::path compared{argv[1]};
  const fs::path reference{argv[2]};
  std::ifstream ifs;
  std::string cmpContent;
  std::string refContent;
  fs::path refPath;
  fs::path relPath;
  fs::path cmpPath;
  for (const auto &ent : fs::recursive_directory_iterator(reference)) {
    if (!ent.is_regular_file())
      continue;
    refPath = ent.path();
    relPath = refPath.lexically_relative(reference);
    cmpPath = compared / relPath;
    if (!fs::exists(cmpPath)) {
      fmt::println("Not found: {}", relPath);
      res = 1;
      continue;
    }
    if (!fs::exists(reference / relPath)) {
      fmt::println("Unexpected output file: {}", relPath);
      res = 1;
      continue;
    }
    if (!(read(cmpPath, ifs, cmpContent) && read(refPath, ifs, refContent))) {
      return 2;
    }
    if (cmpContent != refContent) {
      fmt::println("Mismatched content for: {}", relPath);
      res = 1;
    }
  }
  return res;
}
