#pragma once
#include "utils/Log.hh"
#include <cassert>
#include <llvm/ADT/STLFunctionalExtras.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <string>
#include <system_error>

namespace fs = llvm::sys::fs;

namespace llvm {
class Twine;
class StringRef;
} // namespace llvm

enum class IterDirCtrl : char {
  Continue,
  Stop,    // Return true from iterDir
  ErrStop, // Return false from iterDir (errorneous stop)
};

using IterateDirFn =
    llvm::function_ref<IterDirCtrl(const fs::directory_entry &)>;
namespace {

template <typename It>
[[nodiscard]] bool iterDirImpl(const llvm::Twine &dir,
                               IterateDirFn fn) noexcept {
  std::error_code ec;
  It it{dir, ec}, end;
  if (ec) {
    return err("Error iterating {}: {}", dir, ec.message());
  }
  while (it != end) {
    switch (fn(*it)) {
    case IterDirCtrl::Continue:
      break;
    case IterDirCtrl::Stop:
      return true;
    case IterDirCtrl::ErrStop:
      return false;
    default:
      assert(false);
    }
    it.increment(ec);
    if (ec) {
      return err("Error iterating {}: {}", dir, ec.message());
    }
  }
  return true;
}

} // namespace

// Nicer LLVM's directory iterator.
template <bool recurse = true>
[[nodiscard]] bool iterDir(const llvm::Twine &dir, IterateDirFn fn) noexcept {
  if constexpr (recurse) {
    return iterDirImpl<fs::recursive_directory_iterator>(dir, fn);
  } else {
    return iterDirImpl<fs::directory_iterator>(dir, fn);
  }
}

// Resolve a relative path from a given directory
void mkRelative(std::string &path, const llvm::Twine &dir) noexcept;

// Copy the contents of from into to. Returns true if successful.
[[nodiscard]] bool cpyDir(const llvm::Twine &from,
                          const llvm::Twine &to) noexcept;

enum class CmpRes : char {
  Same,
  Mismatch,
  Err,
};

// Compare the content of 2 files
[[nodiscard]] CmpRes cmpFile(const llvm::Twine &file1,
                             const llvm::Twine &file2) noexcept;

// Compare the content of 2 directories (including file content) and print
// mismatches
[[nodiscard]] CmpRes cmpDir(const llvm::Twine &dir,
                            const llvm::Twine &ref) noexcept;
