// This header acts as a proxy for <doctest/doctest.h> with correct configuration.
// We also replace some macros to work without C++ exceptions, be compatible with LLVM error types
// and ensure fast compilation.

#pragma once
#include <llvm/Support/Error.h>
#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#define DOCTEST_CONFIG_NO_EXCEPTIONS
#define DOCTEST_CONFIG_TREAT_CHAR_STAR_AS_STRING
#define DOCTEST_CONFIG_ASSERTS_RETURN_VALUES
#define DOCTEST_CONFIG_NO_EXCEPTIONS_BUT_WITH_ALL_ASSERTS
#define DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
#include <doctest/doctest.h>
#include <doctest/extensions/doctest_util.h>

#define SUITE DOCTEST_TEST_SUITE_BEGIN
#define END_SUITE DOCTEST_TEST_SUITE_END
#define CASE DOCTEST_TEST_CASE
#define SUBCASE DOCTEST_SUBCASE

// Convenience functions for consuming things and convert it to bool, where true means no error.
// If there's an error (a.k.a this returned false), one must terminate immediately.
auto ok(bool res) noexcept -> bool;
auto ok(llvm::Error err) noexcept -> bool;

template <typename T> auto ok(llvm::Expected<T> &xpected) noexcept -> bool {
  return ok(xpected.takeError());
}

template <typename T> auto ok(llvm::Expected<T> &&xpected) noexcept -> bool {
  return ok(std::move(xpected).takeError());
}

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define CHECK(...) DOCTEST_CHECK_UNARY(ok(__VA_ARGS__))
#define CHECK_FALSE(...) DOCTEST_CHECK_UNARY_FALSE(ok(__VA_ARGS__))
#define CHECK_EQ DOCTEST_CHECK_EQ
#define CHECK_NE DOCTEST_CHECK_NE
#define CHECK_GT DOCTEST_CHECK_GT
#define CHECK_LT DOCTEST_CHECK_LT
#define CHECK_GE DOCTEST_CHECK_GE
#define CHECK_LE DOCTEST_CHECK_LE

// Replacement of REQUIRE* macros via DOCTEST_CONFIG_ASSERTS_RETURN_VALUES.The real REQUIRE*
// with DOCTEST_CONFIG_NO_EXCEPTIONS_BUT_WITH_ALL_ASSERTS behaves like CHECK*.
// NOLINTBEGIN(readability-else-after-return)
#define REQUIRE(...)                                                                               \
  if (!DOCTEST_REQUIRE_UNARY(ok(__VA_ARGS__)))                                                     \
    return;                                                                                        \
  else                                                                                             \
    ((void)0)

#define REQUIRE_FALSE(...)                                                                         \
  if (!DOCTEST_REQUIRE_UNARY_FALSE(ok(__VA_ARGS__)))                                               \
    return;                                                                                        \
  else                                                                                             \
    ((void)0)

#define REQUIRE_EQ(...)                                                                            \
  if (!DOCTEST_REQUIRE_EQ(__VA_ARGS__))                                                            \
    return;                                                                                        \
  else                                                                                             \
    ((void)0)

#define REQUIRE_NE(...)                                                                            \
  if (!DOCTEST_REQUIRE_NE(__VA_ARGS__))                                                            \
    return;                                                                                        \
  else                                                                                             \
    ((void)0)

#define REQUIRE_GT(...)                                                                            \
  if (!DOCTEST_REQUIRE_GT(__VA_ARGS__))                                                            \
    return;                                                                                        \
  else                                                                                             \
    ((void)0)

#define REQUIRE_LT(...)                                                                            \
  if (!DOCTEST_REQUIRE_LT(__VA_ARGS__))                                                            \
    return;                                                                                        \
  else                                                                                             \
    ((void)0)

#define REQUIRE_GE(...)                                                                            \
  if (!DOCTEST_REQUIRE_GE(__VA_ARGS__))                                                            \
    return;                                                                                        \
  else                                                                                             \
    ((void)0)

#define REQUIRE_LE(...)                                                                            \
  if (!DOCTEST_REQUIRE_LE(__VA_ARGS__))                                                            \
    return;                                                                                        \
  else                                                                                             \
    ((void)0)
// NOLINTEND(readability-else-after-return)
// NOLINTEND(cppcoreguidelines-macro-usage)
