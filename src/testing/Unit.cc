#include "testing/Unit.hh"
#include <llvm/ADT/SmallString.h>
#define DOCTEST_CONFIG_IMPLEMENT
#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#define DOCTEST_CONFIG_NO_EXCEPTIONS
#define DOCTEST_CONFIG_NO_WINDOWS_SEH
#define DOCTEST_CONFIG_NO_POSIX_SIGNALS
#include "doctest/doctest.h"
#include "llvm/ADT/StringRef.h"
#include <array>
#include <string>

int runUnitTest(llvm::StringRef name) {
  std::string suiteArg{"--ts="};
  suiteArg += name;
  std::array<const char *, 2> doctestArgs{"run-test unit", suiteArg.data()};
  return doctest::Context(doctestArgs.size(), doctestArgs.data()).run();
}
