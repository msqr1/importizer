#pragma once
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/GlobPattern.h>
#include <optional>

// llvm::GlobPattern with negation
class Glob {
  friend std::optional<Glob> mkGlob(llvm::StringRef) noexcept;
  Glob(llvm::GlobPattern &&ptn, bool negate) noexcept;
  llvm::GlobPattern ptn;
  bool negate;

public:
  [[nodiscard]] bool match(llvm::StringRef s) const noexcept;

  // Disallow copying
  Glob(const Glob &other) noexcept = delete;

  // Allow moving
  Glob(Glob &&other) noexcept;
  Glob &operator=(Glob &&other) noexcept;
};

std::optional<Glob> mkGlob(llvm::StringRef ptn) noexcept;
