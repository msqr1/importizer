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
class StringRef;
} // namespace llvm

auto getRealFS() noexcept -> vfs::FileSystem &;

template <bool recurse>
using DirItT =
    std::conditional_t<recurse, vfs::recursive_directory_iterator, vfs::directory_iterator>;

// Return true to continue, false to stop, and error for well, errorneous stop.
using IterDirCallback = llvm::function_ref<llvm::Expected<bool>(const vfs::directory_entry &)>;

// Nicer LLVM's directory iterator. Returns true if iteration is successful.
template <bool recurse = false>
[[nodiscard]] auto iterDir(const llvm::Twine &dir, IterDirCallback callback,
                           vfs::FileSystem &VFS = getRealFS()) noexcept -> llvm::Error {
  std::error_code errCode;
  DirItT<recurse> it;
  DirItT<recurse> end;
  if constexpr (recurse) {
    it = vfs::recursive_directory_iterator(VFS, dir, errCode);
  } else {
    it = VFS.dir_begin(dir, errCode);
  }
  if (errCode) {
    return llvm::createStringError(errCode,
                                   llvm::formatv("When iterating {}: {}", dir, errCode.message()));
  }
  while (it != end) {
    llvm::Expected<bool> res{callback(*it)};
    if (!res) {
      return res.takeError();
    }
    if (!*res) {
      break;
    }
    it.increment(errCode);
    if (errCode) {
      return llvm::createStringError(
          errCode, llvm::formatv("When iterating {}: {}", dir, errCode.message()));
    }
  }
  return llvm::Error::success();
}

[[nodiscard]] auto cpyDir(const llvm::Twine &src, const llvm::Twine &dst,
                          vfs::FileSystem &srcVFS) noexcept -> llvm::Error;
[[nodiscard]] auto cmpFiles(const llvm::Twine &file1, const llvm::Twine &file2,
                            vfs::FileSystem &file1VFS = getRealFS(),
                            vfs::FileSystem &file2VFS = getRealFS()) noexcept
    -> llvm::Expected<bool>;

// Compare the content of 2 directories (including file content) and print
// mismatches
[[nodiscard]] auto cmpDirs(const llvm::Twine &dir, const llvm::Twine &ref,
                           vfs::FileSystem &dirVFS = getRealFS(),
                           vfs::FileSystem &refVFS = getRealFS()) noexcept -> llvm::Expected<bool>;
