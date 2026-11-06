// This header acts as <doctest/doctest.h> with correct configuration. We also polyfill some macros
// to allow them to work without C++ exceptions.

#pragma once
#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#define DOCTEST_CONFIG_NO_EXCEPTIONS
#define DOCTEST_CONFIG_NO_WINDOWS_SEH
#define DOCTEST_CONFIG_NO_POSIX_SIGNALS
#define DOCTEST_CONFIG_TREAT_CHAR_STAR_AS_STRING
#define DOCTEST_CONFIG_NO_MULTITHREADING
#define DOCTEST_CONFIG_ASSERTS_RETURN_VALUES
#include <doctest/doctest.h>

// Ban syntatic sugar macros that slows down compilation
#undef CHECK
#undef CHECK_FALSE
#undef CHECK_MESSAGE
#undef REQUIRE
#undef REQUIRE_FALSE
#undef REQUIRE_MESSAGE

// Polyfill REQUIRE* macros that rely on DOCTEST_CONFIG_ASSERTS_RETURN_VALUES.
#undef REQUIRE_UNARY
#define REQUIRE_UNARY(...)                                                                         \
  if (!CHECK_UNARY(__VA_ARGS__))                                                                   \
  return

#undef REQUIRE_UNARY_FALSE
#define REQUIRE_UNARY_FALSE(...)                                                                   \
  if (!CHECK_UNARY_FALSE(__VA_ARGS__))                                                             \
  return

#undef REQUIRE_EQ
#define REQUIRE_EQ(...)                                                                            \
  if (!CHECK_EQ(__VA_ARGS__))                                                                      \
  return

#undef REQUIRE_NE
#define REQUIRE_NE(...)                                                                            \
  if (!CHECK_NE(__VA_ARGS__))                                                                      \
  return

#undef REQUIRE_GT
#define REQUIRE_GT(...)                                                                            \
  if (!CHECK_GT(__VA_ARGS__))                                                                      \
  return

#undef REQUIRE_LT
#define REQUIRE_LT(...)                                                                            \
  if (!CHECK_LT(__VA_ARGS__))                                                                      \
  return

#undef REQUIRE_GE
#define REQUIRE_GE(...)                                                                            \
  if (!CHECK_GE(__VA_ARGS__))                                                                      \
  return

#undef REQUIRE_LE
#define REQUIRE_LE(...)                                                                            \
  if (!CHECK_LE(__VA_ARGS__))                                                                      \
  return
