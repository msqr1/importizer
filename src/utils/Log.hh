#pragma once

#include "fmt/base.h"
#include "fmt/color.h"
#include "fmt/format.h"
#include "fmt/ranges.h" // IWYU pragma: keep for formatting ranges
#include "fmt/std.h"    // IWYU pragma: keep for formatting std types
#include <cstdio>
#include <iterator>
#include <string_view>

// Require the program's name to be defined
extern const std::string_view prog;

extern bool raw;

// Must be called before anything
[[nodiscard]] bool getRaw(const int argc, const char **argv) noexcept;

// No newlines added
template <typename... T>
void err(fmt::format_string<T...> fmt, T &&...args) noexcept {
  fmt::memory_buffer buf;

  if (raw) {
    fmt::format_to(std::back_inserter(buf), "{}: error: ", prog);
    fmt::format_to(std::back_inserter(buf), fmt, std::forward<T>(args)...);
  } else {
    fmt::format_to(std::back_inserter(buf), "{}: ", prog);
    fmt::format_to(std::back_inserter(buf),
                   fg(fmt::rgb(0xFF727E)) | fmt::emphasis::bold, "error: ");
    fmt::format_to(std::back_inserter(buf), fmt::emphasis::bold, fmt,
                   std::forward<T>(args)...);
  }
  std::fwrite(buf.data(), sizeof(char), buf.size(), stderr);
}
template <typename... T>
void warn(fmt::format_string<T...> fmt, T &&...args) noexcept {
  fmt::memory_buffer buf;
  if (raw) {
    fmt::format_to(std::back_inserter(buf), "{}: warning: ", prog);
    fmt::format_to(std::back_inserter(buf), fmt, std::forward<T>(args)...);

  } else {
    fmt::format_to(std::back_inserter(buf), "{}: ", prog);
    fmt::format_to(std::back_inserter(buf),
                   fg(fmt::rgb(0xFFAA00)) | fmt::emphasis::bold, "warning: ");
    fmt::format_to(std::back_inserter(buf), fmt::emphasis::bold, fmt,
                   std::forward<T>(args)...);
  }
  std::fwrite(buf.data(), sizeof(char), buf.size(), stderr);
}
