#pragma once
#include <llvm/ADT/StringRef.h>

// Returns true if dir and ref is exactly the same (including file content)
[[nodiscard]] bool cmpDir(llvm::StringRef dir, llvm::StringRef ref) noexcept;
