// The unit test runner. Unit tests are located here.

namespace llvm {
class StringRef;
} // namespace llvm

auto runUnitTest(const char *suiteName, int argc, const char *const *argv) noexcept -> int;
