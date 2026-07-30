#include "utils/Glob.hh"
#include "utils/Log.hh"
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/GlobPattern.h>
#include <optional>
#include <utility>

Glob::Glob(bool negate_, llvm::GlobPattern &&ptn_) noexcept
    : negate{negate_}, ptn{std::move(ptn_)} {}

std::optional<Glob> mkGlob(llvm::StringRef ptn) noexcept {
  bool negate{ptn.consume_front('!')};
  llvm::Expected<llvm::GlobPattern> p{llvm::GlobPattern::create(ptn, {})};
  if (!p) {
    err("Unable to make glob {}", ptn);
    return std::nullopt;
  }
  return std::optional<Glob>(std::in_place, negate, std::move(*p));
}

bool Glob::match(llvm::StringRef s) const noexcept {
  return (ptn.isTrivialMatchAll() || ptn.match(s)) ^ negate;
}
