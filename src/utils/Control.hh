#include "fmt/base.h"
#include "fmt/color.h"
#include "fmt/std.h" // IWYU pragma: keep for formatting fs::path
#include <cstdlib>
#include <string_view>

std::string_view prog();

// No newlines added
template <typename... T> void err(fmt::format_string<T...> fmt, T &&...args) {
  fmt::print(stderr, "{}: ", prog());
  fmt::print(stderr, fg(fmt::rgb(0xFF727E)) | fmt::emphasis::bold, "error: ");
  fmt::print(stderr, fmt::emphasis::bold, fmt, std::forward<T>(args)...);
}
template <typename... T> void warn(fmt::format_string<T...> fmt, T &&...args) {
  fmt::print(stderr, "{}: ", prog());
  fmt::print(stderr, fg(fmt::rgb(0xFFAA00)) | fmt::emphasis::bold, "warning: ");
  fmt::print(stderr, fmt::emphasis::bold, fmt, std::forward<T>(args)...);
}
template <typename... T>
[[noreturn]] void exitWithErr(fmt::format_string<T...> fmt, T &&...args) {
  err(fmt, std::forward<T>(args)...);
  throw EXIT_FAILURE;
}
[[noreturn]] void exitOk();
