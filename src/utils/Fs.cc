#include "utils/Fs.hh"
#include "utils/Log.hh"
#include <cassert>
#include <filesystem>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Twine.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/Path.h>
#include <string>
#include <system_error>
#include <vector>

namespace pth = llvm::sys::path;
namespace fs = llvm::sys::fs;

void mkRelative(std::string &path, const llvm::Twine &dir) noexcept {
  if (!pth::is_relative(path)) {
    return;
  }
  llvm::SmallString<128> tmp;
  dir.toVector(tmp);
  pth::append(tmp, path);
  path.assign(tmp.data(), tmp.size());
}

bool cpyDir(const llvm::Twine &from, const llvm::Twine &to) noexcept {
  llvm::StringRef relPath;
  llvm::SmallString<128> tmp;
  from.toVector(tmp);
  std::error_code ec;
  auto copy{[&](const fs::directory_entry &ent) {
    relPath = ent.path();
    assert(relPath.starts_with(tmp));
    relPath = relPath.drop_front(tmp.size());
    const llvm::Twine dst{to + relPath};
    switch (ent.type()) {
    case fs::file_type::directory_file:
      ec = fs::create_directory(dst);
      break;
    case fs::file_type::regular_file:
      ec = fs::copy_file(ent.path(), dst);
      break;
    case fs::file_type::symlink_file:
      // Falling back to std::filesystem because fs::read_link() won't be here
      // until LLVM 23. TODO: use fs::read_link when LLVM is 23.
      tmp.clear();
      dst.toVector(tmp);
      std::filesystem::copy_symlink(ent.path(), tmp.data(), ec);
      break;
    default: // Ignore
      return IterDirCtrl::Continue;
    }

    if (!ec) {
      err("Error copying {}: {}", ent.path(), ec.message());
      return IterDirCtrl::ErrStop;
    }
    return IterDirCtrl::Continue;
  }};
  return iterDir<true>(from, copy);
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
  llvm::StringRef relPath;
  llvm::SmallString<128> tmp;
  ref.toVector(tmp);
  std::vector<std::string> refRelPaths;

  auto checkRef{[&](const fs::directory_entry &ent) {
    if (ent.type() != fs::file_type::regular_file) {
      return IterDirCtrl::Continue;
    }
    relPath = ent.path();
    assert(relPath.starts_with(tmp));
    relPath = relPath.drop_front(tmp.size());
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
    refRelPaths.emplace_back(relPath);
    return IterDirCtrl::Continue;
  }};
  if (!iterDir(ref, checkRef)) {
    return CmpRes::Err;
  }

  dir.toVector(tmp);
  auto checkDir{[&](const fs::directory_entry &ent) {
    relPath = ent.path();
    assert(relPath.starts_with(tmp));
    relPath = relPath.drop_front(tmp.size());
    bool found{};
    for (llvm::StringRef refRelPath : refRelPaths) {
      if (refRelPath == relPath) {
        found = true;
        break;
      }
    }
    if (!found) {
      err("Unexpected file: {}", relPath);
      res = CmpRes::Mismatch;
    }
    return IterDirCtrl::Continue;
  }};
  return iterDir(dir, checkDir) ? res : CmpRes::Err;
}
