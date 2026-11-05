// llvm::GlobPattern with negation. Enforce that there's only 1 kind of storage
// for globs.

#pragma once
#include "utils/FS.hh"
#include <cassert>
#include <concepts>
#include <cstddef>
#include <llvm/ADT/FunctionExtras.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/GlobPattern.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/VirtualFileSystem.h>
#include <ranges>
#include <utility>
#include <vector>

namespace pth = llvm::sys::path;

class Glob {
  llvm::GlobPattern ptn;
  const bool negate;

public:
  Glob(bool negate, llvm::GlobPattern &&ptn) noexcept;
  Glob(Glob &&) noexcept = default;
  Glob(const Glob &) = delete;
  auto operator=(const Glob &) -> Glob & = delete;

  [[nodiscard]] auto match(llvm::StringRef s) const noexcept -> bool;
};

using GlobStorage = std::vector<Glob>;

auto mkGlob(llvm::StringRef ptn, size_t maxSubpattern = 10) noexcept -> llvm::Expected<Glob>;

template <typename Container, typename ElemT>
concept ConvertibleRange = std::ranges::range<Container> &&
                           std::convertible_to<std::ranges::range_value_t<Container>, ElemT>;

// Make many globs
template <ConvertibleRange<llvm::StringRef> T>
[[nodiscard]] auto mkGlobs(const T &globExprs) noexcept -> llvm::Expected<GlobStorage> {
  GlobStorage globs;
  llvm::Error err{llvm::Error::success()};
  for (llvm::StringRef globExpr : globExprs) {
    llvm::Expected<Glob> g{mkGlob(globExpr)};
    if (!g) {
      err = llvm::joinErrors(std::move(err), g.takeError());
    }
    globs.emplace_back(std::move(*g));
  }
  if (err) {
    return err;
  }
  return globs;
};

struct GlobDirRule {
  GlobStorage globs;
  llvm::unique_function<void(const vfs::directory_entry &ent) const> callback;
};

template <bool recurse = true, ConvertibleRange<GlobDirRule> T>
[[nodiscard]] auto globDir(const llvm::Twine &dir, const T &rules,
                           vfs::FileSystem &dirVFS = getRealFS()) -> llvm::Error {
  llvm::SmallString<64> tmp;
  if (dir.isTriviallyEmpty() || (dir.toVector(tmp), tmp.empty())) {
    return llvm::createStringError("When globbing directory: dir path is empty");
  }
  const size_t baseLen{tmp.size() + static_cast<size_t>(!pth::is_separator(tmp.back()))};
  auto checkDir{[&](const vfs::directory_entry &ent) noexcept -> llvm::Expected<bool> {
    for (const auto &[globs, callback] : rules) {
      for (const Glob &glob : globs) {
        if (glob.match(ent.path().drop_front(baseLen))) {
          callback(ent);
          break;
        }
      }
    }
    return true;
  }};
  return iterDir<recurse>(tmp, checkDir, dirVFS);
}
