#pragma once

#include <cstdlib>
#include <fmt/base.h>
#include <fmt/color.h>
#include <fmt/format.h>
#include <fmt/std.h>

[[noreturn]] void exitOK();
[[noreturn]] void unreachable();

template <typename... T>
[[noreturn]] void exitWithErr(fmt::format_string<T...> f, T &&...args) {
  fmt::print(stderr, fg(fmt::rgb(0xFF727E)) | fmt::emphasis::bold, "Error: ");
  fmt::println(stderr, "{}",
               fmt::format(fmt::emphasis::bold, f, std::forward<T>(args)...));
  throw EXIT_FAILURE;
}
template <typename... T> void warn(fmt::format_string<T...> f, T &&...args) {
  fmt::print(stderr, fg(fmt::rgb(0xFFAA00)) | fmt::emphasis::bold, "Warning: ");
  fmt::println(stderr, "{}",
               fmt::format(fmt::emphasis::bold, f, std::forward<T>(args)...));
}
