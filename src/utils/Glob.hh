#pragma once
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/GlobPattern.h>
#include <optional>

// llvm::GlobPattern with negation
class Glob {
  const bool negate;
  llvm::GlobPattern ptn;

public:
  Glob(bool negate, llvm::GlobPattern &&ptn) noexcept;

  // Disallow copying
  Glob(const Glob &other) = delete;

  // Allow moving
  Glob(Glob &&other) noexcept;

  [[nodiscard]] bool match(llvm::StringRef s) const noexcept;
};

std::optional<Glob> mkGlob(llvm::StringRef ptn) noexcept;
