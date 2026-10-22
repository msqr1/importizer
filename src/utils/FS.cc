#include "utils/FS.hh"
#include "utils/Diag.hh"
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <filesystem>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/StringSet.h>
#include <llvm/ADT/Twine.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/VirtualFileSystem.h>
#include <memory>
#include <system_error>

namespace pth = llvm::sys::path;
namespace fs = llvm::sys::fs;
namespace vfs = llvm::vfs;

auto getRealFS() -> vfs::FileSystem & {
  static std::unique_ptr<vfs::FileSystem> realFS{vfs::createPhysicalFileSystem()};
  return *realFS;
}

auto cpyDir(const llvm::Twine &src, const llvm::Twine &dst, vfs::FileSystem &srcVFS) noexcept
    -> bool {
  llvm::SmallString<128> tmp;
  if (dst.toStringRef(tmp).empty()) {
    return err("Error copying directory: dst path is empty");
  }
  tmp.clear();
  if (src.toStringRef(tmp).empty()) {
    return err("Error copying directory: src path is empty");
  }
  const size_t baseLen{tmp.size() + static_cast<size_t>(!pth::is_separator(tmp.back()))};

  if (std::error_code errCode{fs::create_directories(dst)}) {
    return err("Error making directory {}: {}", dst, errCode.message());
  }

  llvm::StringRef relPath;
  auto copy{[&](const vfs::directory_entry &ent) noexcept -> IterDirCtrl {
    relPath = ent.path().drop_front(baseLen);
    tmp.clear();
    pth::append(tmp, dst, relPath);
    switch (ent.type()) {
    case fs::file_type::directory_file:
      if (std::error_code errCode{fs::create_directory(tmp)}) {
        err("Error making directory {}: {}", tmp, errCode.message());
        return IterDirCtrl::ErrStop;
      }
      break;
    case fs::file_type::regular_file:
      if (std::error_code errCode{fs::copy_file(ent.path(), tmp)}) {
        err("Error copying file {}: {}", tmp, errCode.message());
        return IterDirCtrl::ErrStop;
      }
      break;
    case fs::file_type::symlink_file: {
      // Falling back to std::filesystem because fs::read_link() won't be here
      // until LLVM 23. TODO: use fs::read_link when LLVM is 23.
      std::error_code errCode;
      std::filesystem::copy_symlink(ent.path().data(), tmp.c_str(), errCode);
      if (errCode) {
        err("Error copying symlink {}: {}", ent.path(), errCode.message());
        return IterDirCtrl::ErrStop;
      }
      break;
    }
    default: // Other file types
      break;
    }
    return IterDirCtrl::Continue;
  }};
  return iterDir<true>(tmp, copy, srcVFS);
}

auto cmpFiles(const llvm::Twine &file1, const llvm::Twine &file2, vfs::FileSystem &file1VFS,
              vfs::FileSystem &file2VFS) noexcept -> CmpRes {
  llvm::SmallString<128> tmp;
  if (file1.toStringRef(tmp).empty()) {
    err("Error comparing file: file1 path is empty");
    return CmpRes::Err;
  }
  if (file2.toStringRef(tmp).empty()) {
    err("Error comparing file: file2 path is empty");
    return CmpRes::Err;
  }
  const auto buf1{file1VFS.getBufferForFile(file1, -1, false, false, false)};
  if (!buf1) {
    err("Error opening {}: {}", file1, buf1.getError().message());
    return CmpRes::Err;
  }
  const auto buf2{file2VFS.getBufferForFile(file2, -1, false, false, false)};
  if (!buf2) {
    err("Error opening {}: {}", file2, buf2.getError().message());
    return CmpRes::Err;
  }
  return (**buf1).getBuffer() == (**buf2).getBuffer() ? CmpRes::Same : CmpRes::Mismatch;
}

auto cmpDirs(const llvm::Twine &dir, const llvm::Twine &ref, vfs::FileSystem &dirVFS,
             vfs::FileSystem &refVFS) noexcept -> CmpRes {
  CmpRes res{CmpRes::Same};
  llvm::StringSet<> refRelPaths;
  llvm::SmallString<128> tmp;
  if (dir.toStringRef(tmp).empty()) {
    err("Error comparing directory: dir path is empty");
    return CmpRes::Err;
  }
  const size_t dirBaseLen{tmp.size() + static_cast<size_t>(!pth::is_separator(tmp.back()))};
  tmp.clear();
  if (ref.toStringRef(tmp).empty()) {
    err("Error comparing directory: ref path is empty");
    return CmpRes::Err;
  }
  const size_t refBaseLen{tmp.size() + static_cast<size_t>(!pth::is_separator(tmp.back()))};
  tmp.clear();

  llvm::StringRef relPath;
  auto checkRef{[&](const vfs::directory_entry &ent) noexcept -> IterDirCtrl {
    if (ent.type() != fs::file_type::regular_file) {
      return IterDirCtrl::Continue;
    }
    relPath = ent.path().drop_front(refBaseLen);
    tmp.clear();
    pth::append(tmp, dir, relPath);
    if (!fs::exists(tmp)) {
      err("Missing file: {}", relPath);
      res = CmpRes::Mismatch;
      return IterDirCtrl::Continue;
    }
    switch (cmpFiles(tmp, ent.path(), dirVFS, refVFS)) {
    case CmpRes::Same:
      break;
    case CmpRes::Mismatch:
      err("Mismatched content for {}", relPath);
      res = CmpRes::Mismatch;
      break;
    case CmpRes::Err:
      return IterDirCtrl::ErrStop;
    }
    refRelPaths.insert(relPath);
    tmp.clear();
    return IterDirCtrl::Continue;
  }};
  if (!iterDir<true>(tmp, checkRef, refVFS)) {
    return CmpRes::Err;
  }

  auto checkDir{[&](const vfs::directory_entry &ent) noexcept -> IterDirCtrl {
    if (ent.type() != fs::file_type::regular_file) {
      return IterDirCtrl::Continue;
    }
    relPath = ent.path().drop_front(dirBaseLen);
    if (!refRelPaths.contains(relPath)) {
      err("Unexpected file: {}", relPath);
      res = CmpRes::Mismatch;
    }
    return IterDirCtrl::Continue;
  }};
  return iterDir<true>(dir, checkDir, dirVFS) ? res : CmpRes::Err;
}
