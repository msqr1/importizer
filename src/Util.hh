#pragma once

#include "fmt/base.h"
#include "fmt/color.h"
#include "fmt/format.h"
#include "fmt/std.h" // IWYU pragma: keep for formatting fs::path
#include <cstdlib>

[[noreturn]] void exitOk();
[[noreturn]] void unreachable();

template <typename... T>
[[noreturn]] void exitWithErr(fmt::format_string<T...> f, T &&...args) {
  fmt::println(
      "importizer: {} {}",
      fmt::styled("error:", fg(fmt::rgb(0xFF727E)) | fmt::emphasis::bold),
      fmt::format(fmt::emphasis::bold, f, std::forward<T>(args)...));
  throw EXIT_FAILURE;
}
template <typename... T> void warn(fmt::format_string<T...> f, T &&...args) {
  fmt::println(
      "importizer: {} {}",
      fmt::styled("warning:", fg(fmt::rgb(0xFFAA00)) | fmt::emphasis::bold),
      fmt::format(fmt::emphasis::bold, f, std::forward<T>(args)...));
}
