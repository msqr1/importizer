#include "utils/Fs.hh"
#include "utils/Log.hh"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSet.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include <cassert>
#include <cstddef>
#include <filesystem>
#include <system_error>

namespace pth = llvm::sys::path;
namespace fs = llvm::sys::fs;

bool cpyDir(const llvm::Twine &from, const llvm::Twine &to) noexcept {
  std::error_code ec{fs::create_directories(to)};
  if (ec) {
    return err("Error making directory {}: {}", to, ec.message());
  }
  llvm::StringRef relPath;
  llvm::SmallString<128> tmp;
  from.toVector(tmp);
  size_t baseLen{tmp.size() + !(*tmp.rbegin() == '/' || *tmp.rbegin() == '\\')};
  auto copy{[&](const fs::directory_entry &ent) noexcept {
    relPath = ent.path();
    relPath = relPath.drop_front(baseLen);
    const llvm::Twine dst{to + pth::get_separator() + relPath};
    switch (ent.type()) {
    case fs::file_type::directory_file:
      if ((ec = fs::create_directory(dst))) {
        err("Error making directory {}: {}", dst, ec.message());
        return IterDirCtrl::ErrStop;
      }
      break;
    case fs::file_type::regular_file:
      if ((ec = fs::copy_file(ent.path(), dst))) {
        err("Error copying file {}: {}", ent.path(), ec.message());
        return IterDirCtrl::ErrStop;
      }
      break;
    case fs::file_type::symlink_file:
      // Falling back to std::filesystem because fs::read_link() won't be here
      // until LLVM 23. TODO: use fs::read_link when LLVM is 23.
      tmp.clear();
      dst.toVector(tmp);
      std::filesystem::copy_symlink(ent.path(), tmp.data(), ec);
      if (!ec) {
        err("Error copying symlink {}: {}", ent.path(), ec.message());
        return IterDirCtrl::ErrStop;
      }
      break;
    default: // Other file types
      break;
    }
    return IterDirCtrl::Continue;
  }};
  return iterDir<true>(tmp, copy);
}

CmpRes cmpFile(const llvm::Twine &file1, const llvm::Twine &file2) noexcept {
  const auto buf1{llvm::MemoryBuffer::getFile(file1, false, false, false)};
  if (!buf1) {
    err("Error opening {}: {}", file1, buf1.getError().message());
    return CmpRes::Err;
  }
  const auto buf2{llvm::MemoryBuffer::getFile(file2, false, false, false)};
  if (!buf2) {
    err("Error opening {}: {}", file2, buf2.getError().message());
    return CmpRes::Err;
  }
  return (**buf1).getBuffer() == (**buf2).getBuffer() ? CmpRes::Same
                                                      : CmpRes::Mismatch;
}

CmpRes cmpDir(const llvm::Twine &dir, const llvm::Twine &ref) noexcept {
  CmpRes res{CmpRes::Same};
  llvm::StringSet<> refRelPaths;

  llvm::StringRef relPath;
  llvm::SmallString<128> tmp;
  ref.toVector(tmp);
  size_t baseLen{tmp.size() + !(*tmp.rbegin() == '/' || *tmp.rbegin() == '\\')};
  auto checkRef{[&](const fs::directory_entry &ent) noexcept {
    if (ent.type() != fs::file_type::regular_file) {
      return IterDirCtrl::Continue;
    }
    relPath = ent.path();
    relPath = relPath.drop_front(baseLen);
    const llvm::Twine path{dir + relPath};
    if (!fs::exists(path)) {
      err("Missing file: {}", relPath);
      res = CmpRes::Mismatch;
      return IterDirCtrl::Continue;
    }
    switch (cmpFile(path, ent.path())) {
    case CmpRes::Same:
      break;
    case CmpRes::Mismatch:
      err("Mismatched content for {}", relPath);
      res = CmpRes::Mismatch;
      break;
    case CmpRes::Err:
      return IterDirCtrl::ErrStop;
    }
    refRelPaths.try_emplace(relPath);
    return IterDirCtrl::Continue;
  }};
  if (!iterDir(tmp, checkRef)) {
    return CmpRes::Err;
  }

  tmp.clear();
  dir.toVector(tmp);
  baseLen = tmp.size() + !(*tmp.rbegin() == '/' || *tmp.rbegin() == '\\');
  auto checkDir{[&](const fs::directory_entry &ent) noexcept {
    relPath = ent.path();
    relPath = relPath.drop_front(baseLen);
    if (!refRelPaths.contains(relPath)) {
      err("Unexpected file: {}", relPath);
      res = CmpRes::Mismatch;
    }
    return IterDirCtrl::Continue;
  }};
  return iterDir(tmp, checkDir) ? res : CmpRes::Err;
}
