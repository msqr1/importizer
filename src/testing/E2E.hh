// The end-to-end test runner. End-to-end tests are located in tests/

namespace llvm {
class Twine;
}

bool runE2ETest(const llvm::Twine &testDir, const llvm::Twine &root);
