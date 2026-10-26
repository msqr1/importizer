// llvm::GlobPattern with negation. Enforce that there's only 1 kind of storage
// for globs.

#pragma once
#include "utils/Diag.hh"
#include "utils/FS.hh"
#include <cassert>
#include <concepts>
#include <cstddef>
#include <llvm/ADT/FunctionExtras.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/Support/GlobPattern.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/VirtualFileSystem.h>
#include <optional>
#include <ranges>
#include <utility>
#include <vector>

namespace pth = llvm::sys::path;

namespace llvm {
class StringRef;
} // namespace llvm

class Glob {
  const bool negate;
  llvm::GlobPattern ptn;

public:
  Glob(bool negate, llvm::GlobPattern &&ptn) noexcept;
  Glob(Glob &&) noexcept = default;
  Glob(const Glob &) = delete;
  auto operator=(const Glob &) -> Glob & = delete;

  // Check if the glob matches the given string.
  [[nodiscard]] auto match(llvm::StringRef s) const noexcept -> bool;
};

using GlobStorage = std::vector<Glob>;

auto mkGlob(llvm::StringRef ptn) noexcept -> std::optional<Glob>;

template <typename Container, typename ElemT>
concept ConvertibleRange = std::ranges::range<Container> &&
                           std::convertible_to<std::ranges::range_value_t<Container>, ElemT>;

// Make many globs, return true if none fails.
template <ConvertibleRange<llvm::StringRef> T>
[[nodiscard]] auto mkGlobs(GlobStorage &globs, const T &globExprs) noexcept -> bool {
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
[[nodiscard]] auto globDir(const llvm::Twine &dir, const T &rules,
                           vfs::FileSystem &dirVFS = getRealFS()) -> bool {
  llvm::StringRef relPath;
  llvm::SmallString<128> tmp;
  if (dir.isTriviallyEmpty() || (dir.toVector(tmp), tmp.empty())) {
    return err("Error globbing directory: dir path is empty");
  }
  const size_t baseLen{tmp.size() + static_cast<size_t>(!pth::is_separator(tmp.back()))};
  auto checkDir{[&](const vfs::directory_entry &ent) noexcept -> auto {
    relPath = ent.path().drop_front(baseLen);
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
  return iterDir<true>(tmp, checkDir, dirVFS);
}
