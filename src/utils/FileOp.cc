#include "utils/FileOp.hh"
#include "utils/Log.hh"
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <string>
#ifdef WIN32
#include <array>
#include <string_view>
#else
#include <cerrno>
#endif

namespace fs = std::filesystem;

File portableFOpen(const fs::path &path) noexcept {
  std::FILE *f{};
#ifdef WIN32
  errno_t errCode{_wfopen_s(&f, path.c_str(), L"r")};
  if (errCode != 0) [[unlikely]] {
    std::array<char, 94> msg;
    strerror_s(msg.data(), msg.size(), errCode);
    err("Unable to open {}: {}\n", path, msg.data());
  }
#else
  f = std::fopen(path.c_str(), "r");
  if (f == nullptr) [[unlikely]] {
    err("Unable to open {}: {}\n", path, std::strerror(errno));
  }
#endif
  return File(f);
}

bool readToStr(std::FILE *f, std::string &s, const fs::path &path) noexcept {
  if (std::fseek(f, 0, SEEK_END)) [[unlikely]] {
    err("Unseekable to end: {}\n", path);
    return false;
  }
  const size_t size{static_cast<size_t>(std::ftell(f))};
  if (size == -1) [[unlikely]] {
    err("Unable to get size of {}\n", path);
    return false;
  }
  if (std::fseek(f, 0, 0)) [[unlikely]] {
    err("Unseekable to start: {}\n", path);
    return false;
  }
#ifdef __cpp_lib_string_resize_and_overwrite
  s.resize_and_overwrite(size,
                         [size]([[maybe_unused]] char *_,
                                [[maybe_unused]] size_t _1) { return size; });
#else
  s.resize(size);
#endif
  std::fread(s.data(), sizeof(char), size, f);
  if (std::ferror(f)) [[unlikely]] {
    err("Error occured while reading {}\n", path);
    return false;
  }
  return true;
}

bool readToStr(const fs::path &path, std::string &s) noexcept {
  const File f{portableFOpen(path)};
  return f ? readToStr(f.get(), s, path) : false;
}
