#include "utils/FileOp.hh"
#include "utils/Log.hh"
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <string>
#ifdef _WIN32
#include <array>
#include <stdio.h>
#include <string.h>
#include <string_view>
#include <win32/misc.h>
#else
#include <cerrno>
#endif

namespace fs = std::filesystem;

File openFile(const fs::path &path, std::string_view mode) noexcept {
  std::FILE *f{};
#ifdef _WIN32
  std::array<wchar_t, 3> wMode;
  MultiByteToWideChar(CP_UTF8, 0, mode.data(), mode.size(), wMode.data(),
                      wMode.size());
  int errCode{_wfopen_s(&f, path.c_str(), wMode.data())};
  if (errCode != 0) [[unlikely]] {
    std::array<char, 94> msg;
    strerror_s(msg.data(), msg.size(), errCode);
    err("Unable to open {}: {}.", path, msg.data());
  }
#else
  f = std::fopen(path.c_str(), mode.data());
  if (f == nullptr) [[unlikely]] {
    err("Unable to open {}: {}.", path, std::strerror(errno));
  }
#endif
  return File(f);
}

bool readToStr(std::FILE *f, std::string &s, const fs::path &path) noexcept {
  if (std::fseek(f, 0, SEEK_END)) [[unlikely]] {
    err("Unseekable to end: {}.", path);
    return false;
  }

  const size_t maxSize{static_cast<size_t>(std::ftell(f))};
  if (maxSize == SIZE_MAX) [[unlikely]] {
    err("Unable to get size of {}.", path);
    return false;
  }
  if (std::fseek(f, 0, 0)) [[unlikely]] {
    err("Unseekable to start: {}.", path);
    return false;
  }

  s.resize_and_overwrite(maxSize, [f](char *p, size_t maxSize) {
    // On Windows due to text mode newline translation of \r\n -> \n it can
    // need less than maxSize so we resize accordingly
    return std::fread(p, sizeof(char), maxSize, f);
  });
  if (std::ferror(f)) [[unlikely]] {
    err("Error occured while reading {}.", path);
    return false;
  }

  return true;
}

bool readToStr(const fs::path &path, std::string &s) noexcept {
  const File f{openFile(path)};
  return f && readToStr(f.get(), s, path);
}
