// The unit test runner. Unit tests are located here. This header also acts as doctest with correct
// settings, so it's used instead of #include <doctest/doctest.h>

#pragma once
#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#define DOCTEST_CONFIG_NO_EXCEPTIONS
#define DOCTEST_CONFIG_NO_WINDOWS_SEH
#define DOCTEST_CONFIG_NO_POSIX_SIGNALS
#define DOCTEST_CONFIG_TREAT_CHAR_STAR_AS_STRING
#define DOCTEST_CONFIG_NO_MULTITHREADING
#define DOCTEST_CONFIG_ASSERTS_RETURN_VALUES
#include <doctest/doctest.h>

auto runUnitTest(const char *suiteName, int argc, const char *const *argv) noexcept -> int;
