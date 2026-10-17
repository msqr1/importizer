#pragma once
#include "utils/Log.hh"
#include "llvm/ADT/STLFunctionalExtras.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FileSystem.h"
#include <cassert>
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
  Skip,    // Skip descending into this directory (only for recursion)
};

using IterDirCallback =
    llvm::function_ref<IterDirCtrl(const fs::directory_entry &)>;

template <typename It>
[[nodiscard]] bool iterDirImpl(const llvm::Twine &dir,
                               IterDirCallback callback) noexcept {
  std::error_code ec;
  It it{dir, ec}, end;
  if (ec) {
    return err("Error iterating {}: {}", dir, ec.message());
  }
  while (it != end) {
    switch (callback(*it)) {
    case IterDirCtrl::Continue:
      break;
    case IterDirCtrl::Stop:
      return true;
    case IterDirCtrl::ErrStop:
      return false;
    case IterDirCtrl::Skip:
      if constexpr (std::is_same_v<It, fs::recursive_directory_iterator>) {
        it.no_push();
      } else {
        assert(false && "IterDirCtrl::Skip can't be used for non-recursive "
                        "directory iterator.");
      }
      break;
    }
    it.increment(ec);
    if (ec) {
      return err("Error iterating {}: {}", dir, ec.message());
    }
  }
  return true;
}

// Nicer LLVM's directory iterator. Returns true if iteration is successful.
template <bool recurse = true>
[[nodiscard]] bool iterDir(const llvm::Twine &dir,
                           IterDirCallback callback) noexcept {
  if constexpr (recurse) {
    return iterDirImpl<fs::recursive_directory_iterator>(dir, callback);
  } else {
    return iterDirImpl<fs::directory_iterator>(dir, callback);
  }
}

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
