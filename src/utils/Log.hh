#include "fmt/base.h"
#include "fmt/color.h"
#include "fmt/std.h" // IWYU pragma: keep for formatting fs::path

void progPrefix();

// No newlines added
template <typename... T> void err(fmt::format_string<T...> fmt, T &&...args) {
  progPrefix();
  fmt::print(stderr, fg(fmt::rgb(0xFF727E)) | fmt::emphasis::bold, "error: ");
  fmt::print(stderr, fmt::emphasis::bold, fmt, std::forward<T>(args)...);
}

template <typename... T> void warn(fmt::format_string<T...> fmt, T &&...args) {
  progPrefix();
  fmt::print(stderr, fg(fmt::rgb(0xFFAA00)) | fmt::emphasis::bold, "warning: ");
  fmt::print(stderr, fmt::emphasis::bold, fmt, std::forward<T>(args)...);
}
