#pragma once
#include <llvm/ADT/StringRef.h>

[[nodiscard]] bool cmpDir(const llvm::StringRef &dir,
                          const llvm::StringRef &ref) noexcept;
