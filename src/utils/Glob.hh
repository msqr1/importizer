// llvm::GlobPattern with negation. Enforce that there's only 1 kind of storage
// for globs.

#pragma once
#include "utils/FS.hh"
#include "utils/Types.hh"
#include <llvm/ADT/FunctionExtras.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/GlobPattern.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/VirtualFileSystem.h>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <ranges>
#include <utility>
#include <vector>

namespace pth = llvm::sys::path;

class Glob : MvOnly {
  llvm::GlobPattern ptn_;
  bool const negate_;
  Glob(bool negate, llvm::GlobPattern &&ptn) noexcept;
  friend auto mkGlob(llvm::StringRef ptn, size_t maxSubpattern) noexcept -> llvm::Expected<Glob>;

public:
  [[nodiscard]] auto match(llvm::StringRef s) const noexcept -> bool;
};

using GlobStorage = std::vector<Glob>;

auto mkGlob(llvm::StringRef ptn, size_t maxSubpattern = 1067) noexcept -> llvm::Expected<Glob>;

template <typename Container, typename ElemT>
concept ConvertibleRange = std::ranges::range<Container> &&
                           std::convertible_to<std::ranges::range_value_t<Container>, ElemT>;

// Make many globs
template <ConvertibleRange<llvm::StringRef> T>
auto mkGlobs(T const &globExprs) noexcept -> llvm::Expected<GlobStorage> {
  GlobStorage globs;
  llvm::Error err{llvm::Error::success()};
  for (llvm::StringRef const globExpr : globExprs) {
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
}

struct GlobDirRule {
  GlobStorage globs;
  llvm::unique_function<void(vfs::directory_entry const &ent) const> callback;
};

template <bool Recurse = true, ConvertibleRange<GlobDirRule> T>
auto globDir(llvm::Twine const &dir, T const &rules, vfs::FileSystem &dirVFS = realFS())
    -> llvm::Error {
  llvm::SmallString<64> tmp;
  if (dir.isTriviallyEmpty() || (dir.toVector(tmp), tmp.empty())) {
    return llvm::createStringError("When globbing directory: dir path is empty");
  }
  size_t const baseLen{tmp.size() + static_cast<size_t>(!pth::is_separator(tmp.back()))};
  auto checkDir{[&](vfs::directory_entry const &ent) noexcept -> llvm::Expected<bool> {
    for (auto const &[globs, callback] : rules) {
      for (Glob const &glob : globs) {
        if (glob.match(ent.path().drop_front(baseLen))) {
          callback(ent);
          break;
        }
      }
    }
    return true;
  }};
  return iterDir<Recurse>(tmp, checkDir, dirVFS);
}
