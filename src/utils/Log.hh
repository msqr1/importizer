#pragma once
#include <cstdio>
#include <fmt/base.h>
#include <fmt/color.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/std.h>
#include <iterator>
#include <string>
#include <string_view>
#include <variant>

struct LogOpts {
  bool raw;
  std::string_view prog;
  std::variant<std::FILE *, std::string *> target;
} extern *logOpts;

[[nodiscard]] bool getRaw(bool &raw) noexcept;

template <typename T>
concept BackInsertable = requires(T t) { std::back_inserter(t); };

template <BackInsertable T1, typename... T2>
void fmtErr(T1 &buf, fmt::format_string<T2...> fmt, T2 &&...args) {
  const std::back_insert_iterator<T1> it{std::back_inserter(buf)};
  if (logOpts->raw) {
    fmt::format_to(it, "{}: error: ", logOpts->prog);
    fmt::format_to(it, fmt, std::forward<T2>(args)...);
  } else {
    fmt::format_to(it, "{}: ", logOpts->prog);
    fmt::format_to(it, fg(fmt::rgb(0xFF727E)) | fmt::emphasis::bold, "error: ");
    fmt::format_to(it, fmt::emphasis::bold, fmt, std::forward<T2>(args)...);
  }
  buf.push_back('\n');
}

template <BackInsertable T1, typename... T2>
void fmtWarning(T1 &buf, fmt::format_string<T2...> fmt, T2 &&...args) {
  const std::back_insert_iterator<T1> it{std::back_inserter(buf)};
  if (logOpts->raw) {
    fmt::format_to(it, "{}: warning: ", logOpts->prog);
    fmt::format_to(it, fmt, std::forward<T2>(args)...);

  } else {
    fmt::format_to(it, "{}: ", logOpts->prog);
    fmt::format_to(it, fg(fmt::rgb(0xFFAA00)) | fmt::emphasis::bold,
                   "warning: ");
    fmt::format_to(it, fmt::emphasis::bold, fmt, std::forward<T2>(args)...);
  }
  buf.push_back('\n');
}

template <typename... T>
void err(fmt::format_string<T...> fmt, T &&...args) noexcept {
  if (std::holds_alternative<std::FILE *>(logOpts->target)) {
    fmt::memory_buffer buf;
    fmtErr(buf, fmt, std::forward<T>(args)...);
    std::fwrite(buf.data(), sizeof(char), buf.size(),
                std::get<std::FILE *>(logOpts->target));
    return;
  }
  fmtErr(*std::get<std::string *>(logOpts->target), fmt,
         std::forward<T>(args)...);
}

template <typename... T>
void warn(fmt::format_string<T...> fmt, T &&...args) noexcept {
  if (std::holds_alternative<std::FILE *>(logOpts->target)) {
    fmt::memory_buffer buf;
    fmtWarning(buf, fmt, std::forward<T>(args)...);
    std::fwrite(buf.data(), sizeof(char), buf.size(),
                std::get<std::FILE *>(logOpts->target));
    return;
  }
  fmtWarning(*std::get<std::string *>(logOpts->target), fmt,
             std::forward<T>(args)...);
}
