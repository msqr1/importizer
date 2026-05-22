#include "FileOp.hh"
#include "Util.hh"
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <version>

namespace fs = std::filesystem;
void readFrom(const fs::path &path, std::string &str) {
  std::ifstream ifs{path, std::fstream::binary};
  if (!ifs)
    exitWithErr("Unable to open {} for reading", path);

  // file_size return an uintmax_t which may not be size_t
  size_t fsize{static_cast<size_t>(fs::file_size(path))};
#ifdef __cpp_lib_string_resize_and_overwrite
  str.resize_and_overwrite(
      fsize, [fsize]([[maybe_unused]] char *_, [[maybe_unused]] size_t _1) {
        return fsize;
      });
#else
  str.resize(fsize);
#endif
  ifs.read(str.data(), fsize);
  if (!ifs)
    exitWithErr("Unable to read from {}", path);
#ifdef WIN32

  // Normalize line endings
  std::erase(str, '\r');
#endif
}
void writeTo(const fs::path &path, std::string_view data) {
  fs::create_directories(path.parent_path());
  std::ofstream ofs{path};
  if (!ofs)
    exitWithErr("Unable to open {} for writing", path);
  ofs.write(data.data(), data.length());
  if (!ofs)
    exitWithErr("Unable to write to {}", path);
}
