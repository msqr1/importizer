// Filesystem utils supplementing LLVM's filesystem library. Each function takes
// in default VFS args. This is done for easy unit testing.

#pragma once
#include "utils/Diag.hh"
#include <cassert>
#include <llvm/ADT/STLFunctionalExtras.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/VirtualFileSystem.h>
#include <system_error>
#include <type_traits>

namespace vfs = llvm::vfs;

namespace llvm {
class Twine;
class StringRef;
} // namespace llvm

auto getRealFS() -> vfs::FileSystem &;

enum class IterDirCtrl : char {
  Continue,
  Stop,    // Return true from iterDir (ok stop)
  ErrStop, // Return false from iterDir (errorneous stop)
};

template <bool recurse>
using DirItT =
    std::conditional_t<recurse, vfs::recursive_directory_iterator, vfs::directory_iterator>;

using IterDirCallback = llvm::function_ref<IterDirCtrl(const vfs::directory_entry &)>;

// Nicer LLVM's directory iterator. Returns true if iteration is successful.
template <bool recurse = false>
[[nodiscard]] auto iterDir(const llvm::Twine &dir, IterDirCallback callback,
                           vfs::FileSystem &VFS = getRealFS()) noexcept -> bool {
  std::error_code errCode;
  DirItT<recurse> it;
  DirItT<recurse> end;
  if constexpr (recurse) {
    it = vfs::recursive_directory_iterator(VFS, dir, errCode);
  } else {
    it = VFS.dir_begin(dir, errCode);
  }
  if (errCode) {
    return err("Error iterating {}: {}", dir, errCode.message());
  }
  while (it != end) {
    switch (callback(*it)) {
    case IterDirCtrl::Continue:
      break;
    case IterDirCtrl::Stop:
      return true;
    case IterDirCtrl::ErrStop:
      return false;
    }
    it.increment(errCode);
    if (errCode) {
      return err("Error iterating {}: {}", dir, errCode.message());
    }
  }
  return true;
}

// Copy the contents of src into dst. Returns true if successful. Since
// vfs::FileSystem isn't writable, dst is in the real FS.
[[nodiscard]] auto cpyDir(const llvm::Twine &src, const llvm::Twine &dst,
                          vfs::FileSystem &srcVFS = getRealFS()) noexcept -> bool;

enum class CmpRes : char {
  Same,
  Mismatch,
  Err,
};

[[nodiscard]] auto cmpFiles(const llvm::Twine &file1, const llvm::Twine &file2,
                            vfs::FileSystem &file1VFS = getRealFS(),
                            vfs::FileSystem &file2VFS = getRealFS()) noexcept -> CmpRes;

// Compare the content of 2 directories (including file content) and print
// mismatches
[[nodiscard]] auto cmpDirs(const llvm::Twine &dir, const llvm::Twine &ref,
                           vfs::FileSystem &dirVFS = getRealFS(),
                           vfs::FileSystem &refVFS = getRealFS()) noexcept -> CmpRes;
