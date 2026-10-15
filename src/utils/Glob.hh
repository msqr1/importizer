/// llvm::GlobPattern with negation. We also make it such that there's only 1
/// kind of container for globs.

#pragma once
#include "utils/Fs.hh"
#include <cassert>
#include <concepts>
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/STLFunctionalExtras.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/GlobPattern.h>
#include <optional>
#include <ranges>
#include <utility>
#include <vector>

namespace llvm {
class StringRef;
}

class Glob {
  const bool negate;
  llvm::GlobPattern ptn;

public:
  Glob(bool negate, llvm::GlobPattern &&ptn) noexcept;

  // Allow moving
  Glob(Glob &&) noexcept = default;

  // Disallow copying
  Glob(const Glob &) = delete;
  Glob &operator=(const Glob &) = delete;

  // Check if the glob matches the given string.
  [[nodiscard]] bool match(llvm::StringRef s) const noexcept;
};

using GlobContainer = std::vector<Glob>;

// Make a glob.
std::optional<Glob> mkGlob(llvm::StringRef ptn) noexcept;

template <typename Container, typename ElemT>
concept ConvertibleRange =
    std::ranges::range<Container> &&
    std::convertible_to<std::ranges::range_value_t<Container>, ElemT>;

// Make many globs, return true if none fails.
template <ConvertibleRange<llvm::StringRef> T>
bool mkGlobs(GlobContainer &globs, const T &globExprs) noexcept {
  for (llvm::StringRef globExpr : globExprs) {
    std::optional<Glob> g{mkGlob(globExpr)};
    if (!g) {
      return false;
    }
    globs.emplace_back(std::move(*g));
  }
  return true;
};

using MatchedCallback = llvm::function_ref<void(llvm::StringRef)>;
template <ConvertibleRange<GlobContainer> InT,
          ConvertibleRange<MatchedCallback> OutT>
bool globFromDir(llvm::StringRef dir, const InT &ins, const OutT &callbacks) {
  llvm::StringRef relPath;
  auto checkDir{[&](const fs::directory_entry &ent) noexcept {
    relPath = ent.path();
    assert(relPath.starts_with(dir));
    relPath = relPath.drop_front(dir.size());
    for (const auto &[globs, callback] : llvm::zip_equal(ins, callbacks)) {
      for (const Glob &glob : globs) {
        if (glob.match(relPath)) {
          callback(ent.path());
          break;
        }
      }
    }
    return IterDirCtrl::Continue;
  }};
  return iterDir<true>(dir, checkDir);
}
