/// llvm::GlobPattern with negation. We also make it such that there's only 1
/// kind of container for globs.

#pragma once
#include "utils/Fs.hh"
#include "llvm/ADT/FunctionExtras.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/GlobPattern.h"
#include <cassert>
#include <concepts>
#include <cstddef>
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

using GlobStorage = std::vector<Glob>;

// Make a glob.
std::optional<Glob> mkGlob(llvm::StringRef ptn) noexcept;

template <typename Container, typename ElemT>
concept ConvertibleRange =
    std::ranges::range<Container> &&
    std::convertible_to<std::ranges::range_value_t<Container>, ElemT>;

// Make many globs, return true if none fails.

template <ConvertibleRange<llvm::StringRef> T>
[[nodiscard]] bool mkGlobs(GlobStorage &globs, const T &globExprs) noexcept {
  for (llvm::StringRef globExpr : globExprs) {
    std::optional<Glob> g{mkGlob(globExpr)};
    if (!g) {
      return false;
    }
    globs.emplace_back(std::move(*g));
  }
  return true;
};

struct GlobDirRule {
  GlobStorage globs;
  llvm::unique_function<void(llvm::StringRef) const> callback;
};

template <ConvertibleRange<GlobDirRule> T>
[[nodiscard]] bool globDir(const llvm::Twine &dir, const T &rules) {
  llvm::StringRef relPath;
  llvm::SmallString<128> tmp;
  dir.toVector(tmp);
  size_t baseLen{tmp.size() + !(*tmp.rbegin() == '/' || *tmp.rbegin() == '\\')};
  auto checkDir{[&](const fs::directory_entry &ent) noexcept {
    relPath = ent.path();
    relPath = relPath.drop_front(baseLen);
    for (const auto &[globs, callback] : rules) {
      for (const Glob &glob : globs) {
        if (glob.match(relPath)) {
          callback(ent.path());
          break;
        }
      }
    }
    return IterDirCtrl::Continue;
  }};
  return iterDir<true>(tmp, checkDir);
}
