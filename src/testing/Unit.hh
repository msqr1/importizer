// The unit test runner. Unit tests are located here.

#pragma once
#include <llvm/Support/Error.h>
#include <utility>

// Convenience functions for consuming things and convert it to bool, where true means no error.
// If there's an error (a.k.a this returned false), one must terminate immediately.
auto ok(llvm::Error err) noexcept -> bool;

template <typename T> auto ok(llvm::Expected<T> &xpected) noexcept -> bool {
  return ok(xpected.takeError());
}

template <typename T> auto ok(llvm::Expected<T> &&xpected) noexcept -> bool {
  return ok(std::move(xpected).takeError());
}

auto runUnitTest(const char *suiteName, int argc, const char **argv) noexcept -> int;
