#pragma once
#include <llvm/Support/GlobPattern.h>
#include <optional>

namespace llvm {
class StringRef;
}

// llvm::GlobPattern with negation
class Glob {
  const bool negate;
  llvm::GlobPattern ptn;

public:
  Glob(bool negate, llvm::GlobPattern &&ptn) noexcept;

  // Allow moving
  Glob(Glob &&) noexcept = default;

  // Disallow copying
  Glob(const Glob &) = delete;

  [[nodiscard]] bool match(llvm::StringRef s) const noexcept;
};

std::optional<Glob> mkGlob(llvm::StringRef ptn) noexcept;
