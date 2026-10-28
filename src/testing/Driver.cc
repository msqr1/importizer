#include "testing/E2E.hh"
#include "testing/Unit.hh"
#include <cassert>
#include <cstdlib>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Twine.h>

// run-test E2E [testDir] [root]
// run-test unit [suiteName] -- [doctest args...]
auto main(const int argc, const char *const *argv) -> int {
  llvm::StringRef testType{argv[1]};
  if (argc == 4 && testType == "E2E") {
    return runE2ETest(argv[2], argv[3]) ? EXIT_SUCCESS : EXIT_FAILURE;
  }
  if (argc >= 3 && testType == "unit") {
    return runUnitTest(argv[2], argc - 3, argv + 3);
  }
  return EXIT_FAILURE;
}
