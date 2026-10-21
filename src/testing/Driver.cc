#include "testing/E2E.hh"
#include "testing/Unit.hh"
#include "llvm/ADT/Twine.h"
#include <cassert>
#include <cstdlib>
#include <cstring>

// run-test e2e [testDir] [root]
// run-test unit [name]
int main(const int argc, const char *const *argv) {
  bool E2ETesting{argc == 4 && std::strcmp(argv[1], "E2E")};
  bool unitTesting{argc == 3 && std::strcmp(argv[1], "unit")};

  // Must be one of them
  assert(E2ETesting ^ unitTesting);

  if (E2ETesting) {
    return runE2ETest(argv[2], argv[3]) ? EXIT_SUCCESS : EXIT_FAILURE;
  } else if (unitTesting) {
    return runUnitTest(argv[2]);
  }
}
