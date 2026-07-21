#include "utils/Glob.hh"
#include "utils/Log.hh"
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/GlobPattern.h>
#include <optional>
#include <utility>

Glob::Glob(llvm::GlobPattern &&ptn_, bool negate_) noexcept
    : ptn{std::move(ptn_)}, negate{negate_} {}

Glob::Glob(Glob &&other) noexcept { operator=(std::move(other)); }

Glob &Glob::operator=(Glob &&other) noexcept {
  ptn = std::move(other.ptn);
  negate = other.negate;
  return *this;
}

std::optional<Glob> mkGlob(llvm::StringRef ptn) noexcept {
  bool negate{ptn.consume_front('!')};
  llvm::Expected<llvm::GlobPattern> p{llvm::GlobPattern::create(ptn, {})};
  if (!p) {
    err("Unable to make glob {}", ptn);
    return std::nullopt;
  }
  return Glob{std::move(*p), negate};
}

bool Glob::match(llvm::StringRef s) const noexcept {
  bool res{ptn.isTrivialMatchAll() || ptn.match(s)};
  return negate ? !res : res;
}
