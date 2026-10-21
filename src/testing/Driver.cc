#include "testing/E2E.hh"
#include "testing/Unit.hh"
#include "llvm/ADT/Twine.h"
#include <cassert>
#include <cstdlib>
#include <cstring>

// run-test e2e [testDir] [root]
// run-test unit [suiteName]
int main(const int argc, const char *const *argv) {
  if (argc == 4 && std::strcmp(argv[1], "E2E")) {
    return runE2ETest(argv[2], argv[3]) ? EXIT_SUCCESS : EXIT_FAILURE;
  } else if (argc == 3 && std::strcmp(argv[1], "unit")) {
    return runUnitTest(argv[2]);
  } else {
    return EXIT_FAILURE;
  }
}
