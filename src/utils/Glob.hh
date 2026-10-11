#pragma once
#include <llvm/Support/GlobPattern.h>
#include <optional>
#include <span>
#include <vector>

namespace llvm {
class StringRef;
}

// llvm::GlobPattern with negation
class Glob {
  const bool negate;
  llvm::GlobPattern ptn;

public:
  Glob(bool negate, llvm::GlobPattern &&ptn) noexcept;
  Glob(Glob &&) noexcept = default; // Allow moving
  Glob(const Glob &) = delete;      // Disallow copying

  // Check if the glob matches the given string
  [[nodiscard]] bool match(llvm::StringRef s) const noexcept;
};

// Make a glob
std::optional<Glob> mkGlob(llvm::StringRef ptn) noexcept;

// Make many globs where none can fail
bool mkGlobs(std::vector<Glob> &globs,
             std::span<llvm::StringRef> globExprs) noexcept;
