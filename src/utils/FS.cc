#include "utils/FS.hh"
#include "utils/Diag.hh"
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/StringMap.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Twine.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/VirtualFileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <memory>
#include <system_error>

namespace pth = llvm::sys::path;
namespace fs = llvm::sys::fs;
namespace vfs = llvm::vfs;

auto getRealFS() -> vfs::FileSystem & {
  static std::unique_ptr<vfs::FileSystem> realFS{vfs::createPhysicalFileSystem()};
  return *realFS;
}

auto cmpFiles(const llvm::Twine &file1, const llvm::Twine &file2, vfs::FileSystem &file1VFS,
              vfs::FileSystem &file2VFS) noexcept -> CmpRes {
  llvm::SmallString<128> tmp;
  if (file1.isTriviallyEmpty() || (file1.toVector(tmp), tmp.empty())) {
    err("Error comparing file: file1 path is empty");
    return CmpRes::Err;
  }
  if (file2.isTriviallyEmpty() || (file2.toVector(tmp), tmp.empty())) {
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
  llvm::SmallString<128> tmp;
  if (dir.isTriviallyEmpty() || (dir.toVector(tmp), tmp.empty())) {
    err("Error comparing directory: dir path is empty");
    return CmpRes::Err;
  }
  const size_t dirBaseLen{tmp.size() + static_cast<size_t>(!pth::is_separator(tmp.back()))};
  tmp.clear();
  if (ref.isTriviallyEmpty() || (ref.toVector(tmp), tmp.empty())) {
    err("Error comparing directory: ref path is empty");
    return CmpRes::Err;
  }
  const size_t refBaseLen{tmp.size() + static_cast<size_t>(!pth::is_separator(tmp.back()))};

  llvm::StringMap<fs::file_type> refFileT;
  auto checkRef{[&](const vfs::directory_entry &ent) noexcept -> IterDirCtrl {
    tmp.clear();
    switch (ent.type()) {
    case fs::file_type::regular_file:
    case fs::file_type::directory_file:
      refFileT.try_emplace(ent.path().drop_front(refBaseLen), ent.type());
      break;
    default:
      break;
    }
    return IterDirCtrl::Continue;
  }};
  if (!iterDir<true>(tmp, checkRef, refVFS)) {
    return CmpRes::Err;
  }

  llvm::StringMap<fs::file_type> dirFileT;
  auto checkDir{[&](const vfs::directory_entry &ent) noexcept -> IterDirCtrl {
    switch (ent.type()) {
    case fs::file_type::regular_file:
    case fs::file_type::directory_file:
      dirFileT.try_emplace(ent.path().drop_front(dirBaseLen), ent.type());
      break;
    default:
      break;
    }
    return IterDirCtrl::Continue;
  }};
  if (!iterDir<true>(dir, checkDir, dirVFS)) {
    return CmpRes::Err;
  }

  CmpRes res{CmpRes::Same};
  for (const auto &[relPath, fileT] : refFileT) {
    llvm::StringRef fileTStr{fileT == fs::file_type::regular_file ? "file" : "directory"};
    auto it{dirFileT.find(relPath)};
    if (it == dirFileT.end()) {
      err("Missing {} {}", fileTStr, relPath);
      res = CmpRes::Mismatch;
      continue;
    }
    if (fileT != it->getValue()) {
      err("Expected {} to be a {}", relPath, fileTStr);
      res = CmpRes::Mismatch;
    } else if (fileT == fs::file_type::regular_file) {
      switch (cmpFiles(ref + pth::get_separator() + relPath, dir + pth::get_separator() + relPath,
                       refVFS, dirVFS)) {
      case CmpRes::Same:
        break;
      case CmpRes::Mismatch:
        err("Mismatched file content for {}", relPath);
        res = CmpRes::Mismatch;
        break;
      case CmpRes::Err:
        return CmpRes::Err;
      }
    }
    dirFileT.erase(it);
  }
  for (const auto &[relPath, fileT] : dirFileT) {
    llvm::StringRef fileTStr{fileT == fs::file_type::regular_file ? "file" : "directory"};
    err("Unexpected {} {}", fileTStr, relPath);
    res = CmpRes::Mismatch;
  }
  return res;
}
