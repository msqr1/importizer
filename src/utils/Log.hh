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

template <typename... Ts> struct Overload : Ts... {
  using Ts::operator()...;
};

template <typename T>
concept BackInsertable = requires(T t) { std::back_inserter(t); };

template <BackInsertable T, typename... Ts>
void fmtErr(T *buf, fmt::format_string<Ts...> fmt, Ts &&...args) {
  const std::back_insert_iterator<T> it{std::back_inserter(*buf)};
  if (logOpts->raw) {
    fmt::format_to(it, "{}: error: ", logOpts->prog);
    fmt::format_to(it, fmt, std::forward<Ts>(args)...);
  } else {
    fmt::format_to(it, "{}: ", logOpts->prog);
    fmt::format_to(it, fg(fmt::rgb(0xFF727E)) | fmt::emphasis::bold, "error: ");
    fmt::format_to(it, fmt::emphasis::bold, fmt, std::forward<Ts>(args)...);
  }
  buf->push_back('\n');
}

template <typename... Ts>
void err(fmt::format_string<Ts...> fmt, Ts &&...args) noexcept {
  Overload ov{
      [&](std::FILE *target) {
        fmt::memory_buffer buf;
        fmtErr(&buf, fmt, std::forward<Ts>(args)...);
        std::fwrite(buf.data(), sizeof(char), buf.size(), target);
      },
      [&](std::string *target) {
        fmtErr(target, fmt, std::forward<Ts>(args)...);
      },
  };
  std::visit(ov, logOpts->target);
}

template <BackInsertable T, typename... Ts>
void fmtWarning(T *buf, fmt::format_string<Ts...> fmt, Ts &&...args) {
  const std::back_insert_iterator<T> it{std::back_inserter(*buf)};
  if (logOpts->raw) {
    fmt::format_to(it, "{}: warning: ", logOpts->prog);
    fmt::format_to(it, fmt, std::forward<Ts>(args)...);
  } else {
    fmt::format_to(it, "{}: ", logOpts->prog);
    fmt::format_to(it, fg(fmt::rgb(0xFFAA00)) | fmt::emphasis::bold,
                   "warning: ");
    fmt::format_to(it, fmt::emphasis::bold, fmt, std::forward<Ts>(args)...);
  }
  buf->push_back('\n');
}

template <typename... Ts>
void warn(fmt::format_string<Ts...> fmt, Ts &&...args) noexcept {
  Overload ov{
      [&](std::FILE *target) {
        fmt::memory_buffer buf;
        fmtWarning(&buf, fmt, std::forward<Ts>(args)...);
        std::fwrite(buf.data(), sizeof(char), buf.size(), target);
      },
      [&](std::string *target) {
        fmtWarning(target, fmt, std::forward<Ts>(args)...);
      },
  };
  std::visit(ov, logOpts->target);
}
