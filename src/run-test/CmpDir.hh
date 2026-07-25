#pragma once

namespace llvm {
class StringRef;
}

// Returns true if dir & ref is exactly the same (including file content)
[[nodiscard]] bool cmpDir(llvm::StringRef dir, llvm::StringRef ref) noexcept;
