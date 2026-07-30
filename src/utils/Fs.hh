#pragma once
#include "utils/Log.hh"
#include <llvm/ADT/STLFunctionalExtras.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <system_error>
#include <utility>

namespace fs = llvm::sys::fs;
namespace pth = llvm::sys::path;
namespace llvm {
class Twine;
}

namespace detail {

template <typename It>
[[nodiscard]] bool
iterateDir(const llvm::Twine &dir,
           llvm::function_ref<bool(const fs::directory_entry &)> fn) noexcept {
  std::error_code ec;
  It it{dir, ec}, end;
  if (ec) {
    return err("Unable to iterate {}: {}", dir, ec.message());
  }
  while (it != end) {
    if (!fn(*it)) {
      return false;
    }
    it.increment(ec);
    if (ec) {
      return err("Unable to iterate {}: {}", dir, ec.message());
    }
  }
  return true;
}

} // namespace detail

// Nicer LLVM's directory iterator. fn should return true to continue iterating.
template <bool recurse = true>
[[nodiscard]] bool
iterateDir(const llvm::Twine &dir,
           llvm::function_ref<bool(const fs::directory_entry &)> fn) noexcept {
  if constexpr (recurse) {
    return detail::iterateDir<fs::recursive_directory_iterator>(dir, fn);
  } else {
    return detail::iterateDir<fs::directory_iterator>(dir, fn);
  }
}

template <unsigned len>
void makeRelative(llvm::SmallString<len> &path, const llvm::Twine &dir) {
  if (!pth::is_relative(path)) {
    return;
  }
  llvm::SmallString<len> tmp;
  dir.toVector(tmp);
  pth::append(tmp, path);
  path = std::move(tmp);
}
