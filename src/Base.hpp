#pragma once
#include <fmt/base.h>
#include <fmt/std.h>
#include <fmt/format.h>
#include <cstddef>
#include <source_location>
#include <filesystem>

// PCRE2 for non-matching capture groups, and
// std::string(_view)::find* for not found return a -1 size_t (std::string::npos)
constexpr size_t notFound{static_cast<size_t>(-1)};
[[noreturn]] void exitOK();
[[noreturn]] void unreachable();
using fmt::print;
using fmt::println;
using fmt::format;
template <typename OutputIt, typename... T> void formatTo(OutputIt&& out,
  fmt::format_string<T...> fmt, T&&... args) {
  fmt::format_to(std::forward<OutputIt>(out), fmt, std::forward<T>(args)...);
}
template <typename... T> struct exitWithErr {

  // Overload for direct callers, source location is implied
  [[noreturn]] exitWithErr(fmt::format_string<T...> fmt, T&&... args, 
  const std::source_location& loc = std::source_location::current()) {
    exitWithErr(loc, fmt, std::forward<T>(args)...);

    // Idk why but my compiler warns on the noreturn function returning without this,
    // even if its literally unreachable
    unreachable();
  }
  
  // Overload for indirect callers (ie. calling from an error handling function). 
  // Custom source location is here to give correct error location
  [[noreturn]] exitWithErr(const std::source_location& loc, fmt::format_string<T...> fmt
    , T&&... args) {
    print(stderr, "Exception thrown at {}({}:{}): ",
      std::filesystem::path(loc.file_name()).filename(), loc.line(), loc.column());
    println(stderr, fmt, std::forward<T>(args)...);
    throw 1;
  }
};
template <typename... T> exitWithErr(fmt::format_string<T...> fmt, T&&...)
  -> exitWithErr<T...>;
template <typename... T> exitWithErr(const std::source_location& loc,
  fmt::format_string<T...> fmt, T&&...) -> exitWithErr<T...>;
