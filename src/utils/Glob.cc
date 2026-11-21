#include "utils/Glob.hh"
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/GlobPattern.h>
#include <cstddef>
#include <utility>

Glob::Glob(bool const negate, llvm::GlobPattern &&ptn) noexcept
    : ptn_{std::move(ptn)}, negate_{negate} {}

auto mkGlob(llvm::StringRef ptn, size_t maxSubpattern) noexcept -> llvm::Expected<Glob> {
  bool const negate{ptn.consume_front('!')};
  llvm::Expected<llvm::GlobPattern> p{llvm::GlobPattern::create(ptn, maxSubpattern)};
  if (!p) {
    return p.takeError();
  }
  return Glob{negate, std::move(*p)};
}

auto Glob::match(llvm::StringRef str) const noexcept -> bool {
  return (ptn_.isTrivialMatchAll() || ptn_.match(str)) ^ negate_;
}
