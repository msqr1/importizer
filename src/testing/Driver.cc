#include "testing/E2E.hh"
#include "testing/Unit.hh"
#include "utils/Diag.hh"
#include <cassert>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Twine.h>
#include <llvm/Support/Error.h>

// test-tool E2E [testDir] [root]
// test-tool unit [suiteName] -- [doctest args...]
auto main(const int argc, const char *const *argv) -> int {
  llvm::StringRef testType{argv[1]};
  if (argc == 4 && testType == "E2E") {
    llvm::handleAllErrors(runE2ETest(argv[2], argv[3]),
                          [](const llvm::ErrorInfoBase &errInfo) -> llvm::Error {
                            if (!errInfo.isA<EmptyErr>()) {
                              err("{}", errInfo.message());
                            }
                            return llvm::Error::success();
                          });
  }
  if (argc >= 3 && testType == "unit") {
    return runUnitTest(argv[2], argc - 3, argv + 3);
  }
}
