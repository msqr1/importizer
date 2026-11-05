// The end-to-end test runner. End-to-end tests are located in tests/

#include <llvm/Support/Error.h>

namespace llvm {
class Twine;
} // namespace llvm

/*
my-new-feature/     # Directory named after the test (testDir)
├── Config.yml      # importizer's configuration
├── in/             # Input files
├── ref/            # Expected output files (optional)
└── RefCLI.txt      # Expected CLI output
*/

// Run the end-to-end test and log defects.
auto runE2ETest(const llvm::Twine &testDir, const llvm::Twine &outDir) noexcept -> llvm::Error;

// Run end-to-end but update ref/
auto updateE2ETest(const llvm::Twine &testDir) noexcept -> llvm::Error;
