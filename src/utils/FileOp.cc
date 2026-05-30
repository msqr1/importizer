#include "utils/FileOp.hh"
#include "utils/Log.hh"
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <string>
#ifdef WIN32
#include <win32/misc.h>
#include <array>
#include <string_view>
#include <stdio.h>
#include <string.h>
#else
#include <cerrno>
#endif

namespace fs = std::filesystem;

File portableFOpen(const fs::path &path, std::string_view mode) noexcept {
  std::FILE *f{};
#ifdef WIN32
  std::array<wchar_t, 3> wMode;
  MultiByteToWideChar(CP_UTF8, 0, mode.data(), mode.size(), wMode.data(), wMode.size());
  int errCode{_wfopen_s(&f, path.c_str(), wMode.data())};
  if (errCode != 0) [[unlikely]] {
    std::array<char, 94> msg;
    strerror_s(msg.data(), msg.size(), errCode);
    err("Unable to open {}: {}", path, msg.data());
  }
#else
  f = std::fopen(path.c_str(), mode.data);
  if (f == nullptr) [[unlikely]] {
    err("Unable to open {}: {}", path, std::strerror(errno));
  }
#endif
  return File(f);
}

bool readToStr(std::FILE *f, std::string &s, const fs::path &path) noexcept {
  if (std::fseek(f, 0, SEEK_END)) [[unlikely]] {
    err("Unseekable to end: {}", path);
    return false;
  }
  const size_t size{static_cast<size_t>(std::ftell(f))};
  if (size == SIZE_MAX) [[unlikely]] {
    err("Unable to get size of {}", path);
    return false;
  }
  if (std::fseek(f, 0, 0)) [[unlikely]] {
    err("Unseekable to start: {}", path);
    return false;
  }
  s.resize_and_overwrite(size, [size](char *, size_t) { return size - 1; });
  std::fread(s.data(), sizeof(char), size, f);
  if (std::ferror(f)) [[unlikely]] {
    err("Error occured while reading {}", path);
    return false;
  }
  return true;
}

bool readToStr(const fs::path &path, std::string &s) noexcept {
  const File f{portableFOpen(path)};
  return f ? readToStr(f.get(), s, path) : false;
}
