// This header acts as <doctest/doctest.h> with correct configuration. We also replace some macros
// to work without C++ exceptions.

#pragma once
#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#define DOCTEST_CONFIG_NO_EXCEPTIONS
#define DOCTEST_CONFIG_NO_WINDOWS_SEH
#define DOCTEST_CONFIG_NO_POSIX_SIGNALS
#define DOCTEST_CONFIG_TREAT_CHAR_STAR_AS_STRING
#define DOCTEST_CONFIG_NO_MULTITHREADING
#define DOCTEST_CONFIG_ASSERTS_RETURN_VALUES
#define DOCTEST_CONFIG_NO_EXCEPTIONS_BUT_WITH_ALL_ASSERTS
#include <doctest/doctest.h>

// Ban syntatic sugar macros that slows down compilation
#undef REQUIRE
#undef REQUIRE_FALSE
#undef REQUIRE_MESSAGE
#undef CHECK
#undef CHECK_FALSE
#undef CHECK_MESSAGE
#undef WARN
#undef WARN_FALSE
#undef WARN_MESSAGE

// Replace REQUIRE_* macros that rely on DOCTEST_CONFIG_ASSERTS_RETURN_VALUES.The real REQUIRE_*
// with DOCTEST_CONFIG_NO_EXCEPTIONS_BUT_WITH_ALL_ASSERTS behaves like CHECK_*.
#undef REQUIRE_UNARY
#define REQUIRE_UNARY(...)                                                                         \
  if (!DOCTEST_REQUIRE_UNARY(__VA_ARGS__))                                                         \
  return

#undef REQUIRE_UNARY_FALSE
#define REQUIRE_UNARY_FALSE(...)                                                                   \
  if (!DOCTEST_REQUIRE_UNARY_FALSE(__VA_ARGS__))                                                   \
  return

#undef REQUIRE_EQ
#define REQUIRE_EQ(...)                                                                            \
  if (!DOCTEST_REQUIRE_EQ(__VA_ARGS__))                                                            \
  return

#undef REQUIRE_NE
#define REQUIRE_NE(...)                                                                            \
  if (!DOCTEST_REQUIRE_NE(__VA_ARGS__))                                                            \
  return

#undef REQUIRE_GT
#define REQUIRE_GT(...)                                                                            \
  if (!DOCTEST_REQUIRE_GT(__VA_ARGS__))                                                            \
  return

#undef REQUIRE_LT
#define REQUIRE_LT(...)                                                                            \
  if (!DOCTEST_REQUIRE_LT(__VA_ARGS__))                                                            \
  return

#undef REQUIRE_GE
#define REQUIRE_GE(...)                                                                            \
  if (!DOCTEST_REQUIRE_GE(__VA_ARGS__))                                                            \
  return

#undef REQUIRE_LE
#define REQUIRE_LE(...)                                                                            \
  if (!DOCTEST_REQUIRE_LE(__VA_ARGS__))                                                            \
  return
