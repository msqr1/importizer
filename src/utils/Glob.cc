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
    : ptn{std::move(ptn_)}, negate{negate_} {}

auto mkGlob(llvm::StringRef ptn, size_t maxSubpattern) noexcept -> llvm::Expected<Glob> {
  bool negate{ptn.consume_front('!')};
  llvm::Expected<llvm::GlobPattern> p{llvm::GlobPattern::create(ptn, maxSubpattern)};
  if (!p) {
    return p.takeError();
  }
  return Glob{negate, std::move(*p)};
}

auto Glob::match(llvm::StringRef str) const noexcept -> bool {
  return (ptn.isTrivialMatchAll() || ptn.match(str)) ^ negate;
}
