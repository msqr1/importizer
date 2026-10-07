#pragma once
#include "utils/Log.hh"
#include <llvm/ADT/STLFunctionalExtras.h>
#include <llvm/Support/FileSystem.h>
#include <string>
#include <system_error>

namespace fs = llvm::sys::fs;

namespace llvm {
class Twine;
}

namespace {

template <typename It>
[[nodiscard]] bool iterateDirImpl(
    const llvm::Twine &dir,
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

} // namespace

// Nicer LLVM's directory iterator. fn should return true to continue iterating.
template <bool recurse = true>
[[nodiscard]] bool
iterateDir(const llvm::Twine &dir,
           llvm::function_ref<bool(const fs::directory_entry &)> fn) noexcept {
  if constexpr (recurse) {
    return iterateDirImpl<fs::recursive_directory_iterator>(dir, fn);
  } else {
    return iterateDirImpl<fs::directory_iterator>(dir, fn);
  }
}

void mkRelative(std::string &path, const llvm::Twine &dir);
