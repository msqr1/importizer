// Filesystem utils supplementing LLVM's filesystem library. Each function takes
// in default VFS args. This is done for easy unit testing.

#pragma once
#include "utils/Diag.hh"
#include "llvm/ADT/STLFunctionalExtras.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/VirtualFileSystem.h"
#include <cassert>
#include <system_error>
#include <type_traits>

namespace vfs = llvm::vfs;

namespace llvm {
class Twine;
class StringRef;
} // namespace llvm

enum class IterDirCtrl : char {
  Continue,
  Stop,    // Return true from iterDir (ok stop)
  ErrStop, // Return false from iterDir (errorneous stop)
};

template <bool recurse>
using DirItT = std::conditional_t<recurse, vfs::recursive_directory_iterator,
                                  vfs::directory_iterator>;

using IterDirCallback =
    llvm::function_ref<IterDirCtrl(const vfs::directory_entry &)>;

// Nicer LLVM's directory iterator. Returns true if iteration is successful.
template <bool recurse = false>
[[nodiscard]] bool
iterDir(const llvm::Twine &dir, IterDirCallback callback,
        vfs::FileSystem &VFS = *vfs::createPhysicalFileSystem()) noexcept {
  std::error_code ec;
  DirItT<recurse> it, end;
  if constexpr (recurse) {
    it = vfs::recursive_directory_iterator(VFS, dir, ec);
  } else {
    it = VFS.dir_begin(dir, ec);
  }
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

// Copy the contents of src into dst. Returns true if successful. Since
// vfs::FileSystem isn't writable, dst is in the real FS.
[[nodiscard]] bool
cpyDir(const llvm::Twine &src, const llvm::Twine &dst,
       vfs::FileSystem &srcVFS = *vfs::createPhysicalFileSystem()) noexcept;

enum class CmpRes : char {
  Same,
  Mismatch,
  Err,
};

[[nodiscard]] CmpRes
cmpFile(const llvm::Twine &file1, const llvm::Twine &file2,
        vfs::FileSystem &file1VFS = *vfs::createPhysicalFileSystem(),
        vfs::FileSystem &file2VFS = *vfs::createPhysicalFileSystem()) noexcept;

// Compare the content of 2 directories (including file content) and print
// mismatches
[[nodiscard]] CmpRes
cmpDir(const llvm::Twine &dir, const llvm::Twine &ref,
       vfs::FileSystem &dirVFS = *vfs::createPhysicalFileSystem(),
       vfs::FileSystem &refVFS = *vfs::createPhysicalFileSystem()) noexcept;
