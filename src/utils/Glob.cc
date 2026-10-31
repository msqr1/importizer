#include "utils/Glob.hh"
#include "utils/Diag.hh"
#include <cstddef>
#include <llvm/ADT/FunctionExtras.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/GlobPattern.h>
#include <optional>
#include <utility>

Glob::Glob(const bool negate_, llvm::GlobPattern &&ptn_) noexcept
    : negate{negate_}, ptn{std::move(ptn_)} {}

auto mkGlob(llvm::StringRef ptn, size_t maxSubpattern) noexcept -> std::optional<Glob> {
  bool negate{ptn.consume_front('!')};
  llvm::Expected<llvm::GlobPattern> p{llvm::GlobPattern::create(ptn, maxSubpattern)};
  if (!p) {
    err("Error making glob {}: {}", ptn, llvm::toString(p.takeError()));
    return std::nullopt;
  }
  return std::optional<Glob>(std::in_place, negate, std::move(*p));
}

auto Glob::match(llvm::StringRef str) const noexcept -> bool {
  return (ptn.isTrivialMatchAll() || ptn.match(str)) ^ negate;
}
