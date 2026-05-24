#include "utils/FileOp.hh"
#include "utils/Control.hh"
#include <array> // IWYU pragma: keep for Windows
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

File portableFOpen(const fs::path &path) {
  std::FILE *f;
#ifdef WIN32
  errno_t errCode{_wfopen_s(&f, path.c_str(), L"r")};
  if (errCode != 0) {
    std::array<char, 128> msg;
    strerror_s(msg, msg.size(), errCode);
    exitWithErr("Unable to open {}: {}\n", path, msg.data());
    return nullptr;
  }
#else
  f = std::fopen(path.c_str(), "r");
  if (f == nullptr) {
    exitWithErr("Unable to open {}: {}\n", path, std::strerror(errno));
  }
#endif
  return File(f);
}

void readToStr(std::FILE *f, std::string &s, const fs::path &path) {
  std::fseek(f, 0, SEEK_END);
  const long len{std::ftell(f)};
  if (len == -1) {
    err("Unable to get size of {}\n", path);
  }
#ifdef __cpp_lib_string_resize_and_overwrite
  s.resize_and_overwrite(len,
                         [len]([[maybe_unused]] char *_,
                               [[maybe_unused]] size_t _1) { return len; });
#else
  s.resize(len);
#endif
  std::fread(s.data(), 1, len, f);
  if (std::ferror(f)) {
    exitWithErr("Error occured while reading {}\n", path);
  }
}

void readToStr(const fs::path &path, std::string &s) {
  File f{portableFOpen(path)};
  readToStr(f.get(), s, path);
}
