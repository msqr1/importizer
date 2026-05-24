#pragma once

#include "fmt/base.h"
#include "utils/Log.hh"
#include <cstdlib>

template <typename... T>
[[noreturn]] void exitWithErr(fmt::format_string<T...> fmt, T &&...args) {
  err(fmt, std::forward<T>(args)...);
  throw EXIT_FAILURE;
}
[[noreturn]] void exitOk();
