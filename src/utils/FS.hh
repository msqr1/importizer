// Filesystem utils supplementing LLVM's filesystem library. Each function takes
// in default VFS args. This is done for easy unit testing.

#pragma once
#include <cassert>
#include <llvm/ADT/STLFunctionalExtras.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/ErrorOr.h>
#include <llvm/Support/FormatVariadic.h>
#include <llvm/Support/VirtualFileSystem.h>
#include <system_error>
#include <type_traits>

namespace vfs = llvm::vfs;

namespace llvm {
class Twine;
} // namespace llvm

auto realFS() noexcept -> vfs::FileSystem &;

template <bool Recurse>
using DirItT =
    std::conditional_t<Recurse, vfs::recursive_directory_iterator, vfs::directory_iterator>;

// Return true to continue, false to stop, and error for well, errorneous stop.
using IterDirCallback = llvm::function_ref<llvm::Expected<bool>(const vfs::directory_entry &)>;

// Nicer LLVM's directory iterator. Returns true if iteration is successful.
template <bool Recurse = false>
auto iterDir(const llvm::Twine &dir, IterDirCallback callback,
             vfs::FileSystem &VFS = realFS()) noexcept -> llvm::Error {
  std::error_code err;
  DirItT<Recurse> it;
  DirItT<Recurse> end;
  if constexpr (Recurse) {
    it = vfs::recursive_directory_iterator(VFS, dir, err);
  } else {
    it = VFS.dir_begin(dir, err);
  }
  if (err) {
    return llvm::createStringError(err, llvm::formatv("When iterating {}: {}", dir, err.message()));
  }
  while (it != end) {
    llvm::Expected<bool> res{callback(*it)};
    if (!res) {
      return res.takeError();
    }
    if (!*res) {
      break;
    }
    it.increment(err);
    if (err) {
      return llvm::createStringError(err,
                                     llvm::formatv("When iterating {}: {}", dir, err.message()));
    }
  }
  return llvm::Error::success();
}

auto cpyDir(const llvm::Twine &src, const llvm::Twine &dst,
            vfs::FileSystem &srcVFS = realFS()) noexcept -> llvm::Error;

auto cmpFiles(const llvm::Twine &file1, const llvm::Twine &file2,
              vfs::FileSystem &file1VFS = realFS(), vfs::FileSystem &file2VFS = realFS()) noexcept
    -> llvm::Expected<bool>;

// Compare the content of 2 directories (including file content) and log mismatches as errors.
auto cmpDirs(const llvm::Twine &dir, const llvm::Twine &ref, vfs::FileSystem &dirVFS = realFS(),
             vfs::FileSystem &refVFS = realFS()) noexcept -> llvm::Expected<bool>;
