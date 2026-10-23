// The end-to-end test runner. End-to-end tests are located in tests/

namespace llvm {
class Twine;
} // namespace llvm

auto runE2ETest(const llvm::Twine &testDir, const llvm::Twine &root) noexcept -> bool;
