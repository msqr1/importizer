#include "utils/FS.hh"
#include "utils/Diag.hh"
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/StringMap.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Twine.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/FormatVariadic.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/VirtualFileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <memory>
#include <system_error>

namespace pth = llvm::sys::path;
namespace fs = llvm::sys::fs;
namespace vfs = llvm::vfs;

auto getRealFS() noexcept -> vfs::FileSystem & {
  static std::unique_ptr<vfs::FileSystem> realFS{vfs::createPhysicalFileSystem()};
  return *realFS;
}

auto cpyDir(const llvm::Twine &src, const llvm::Twine &dst, vfs::FileSystem &srcVFS) noexcept
    -> llvm::Error {
  llvm::SmallString<64> tmp;
  if (dst.isTriviallyEmpty() || (dst.toVector(tmp), tmp.empty())) {
    return llvm::createStringError("When copying directory: dst path is empty.");
  }
  tmp.clear();
  if (src.isTriviallyEmpty() || (src.toVector(tmp), tmp.empty())) {
    return llvm::createStringError("When copying directory: src path is empty.");
  }
  const size_t baseLen{tmp.size() + static_cast<size_t>(!pth::is_separator(tmp.back()))};
  if (std::error_code err{fs::create_directories(dst)}) {
    return llvm::createStringError(
        err, llvm::formatv("When creating directory {}: {}.", dst, err.message()));
  }
  auto copy{[&](const vfs::directory_entry &ent) noexcept -> llvm::Expected<bool> {
    llvm::StringRef relPath{ent.path().drop_front(baseLen)};
    tmp.clear();
    pth::append(tmp, dst, relPath);
    switch (ent.type()) {
    case fs::file_type::directory_file:
      if (std::error_code err{fs::create_directory(tmp)}) {
        return llvm::createStringError(
            err, llvm::formatv("When creating directory {}: {}.", tmp, err.message()));
      }
      break;
    case fs::file_type::regular_file:
      if (std::error_code err{fs::copy_file(ent.path(), tmp)}) {
        return llvm::createStringError(
            err, llvm::formatv("When copying file {}: {}.", ent.path(), err.message()));
      }
      break;
    default: // Other file types
      break;
    }
    return true;
  }};
  return iterDir<true>(tmp, copy, srcVFS);
}

auto cmpFiles(const llvm::Twine &file1, const llvm::Twine &file2, vfs::FileSystem &file1VFS,
              vfs::FileSystem &file2VFS) noexcept -> llvm::Expected<bool> {
  const auto buf1{file1VFS.getBufferForFile(file1, -1, false, false, false)};
  if (std::error_code err{buf1.getError()}) {
    return llvm::createStringError(err, llvm::formatv("When opening {}: {}", file1, err.message()));
  }
  const auto buf2{file2VFS.getBufferForFile(file2, -1, false, false, false)};
  if (std::error_code err{buf2.getError()}) {
    return llvm::createStringError(err, llvm::formatv("When opening {}: {}", file2, err.message()));
  }
  return (**buf1).getBuffer() == (**buf2).getBuffer();
}

auto cmpDirs(const llvm::Twine &dir, const llvm::Twine &ref, vfs::FileSystem &dirVFS,
             vfs::FileSystem &refVFS) noexcept -> llvm::Expected<bool> {
  llvm::SmallString<64> tmp;
  if (dir.isTriviallyEmpty() || (dir.toVector(tmp), tmp.empty())) {
    return llvm::createStringError("When comparing directories: dir path is empty");
  }
  const size_t dirBaseLen{tmp.size() + static_cast<size_t>(!pth::is_separator(tmp.back()))};
  tmp.clear();
  if (ref.isTriviallyEmpty() || (ref.toVector(tmp), tmp.empty())) {
    return llvm::createStringError("When comparing directories: ref path is empty");
  }
  const size_t refBaseLen{tmp.size() + static_cast<size_t>(!pth::is_separator(tmp.back()))};

  llvm::StringMap<fs::file_type> refFileT;
  auto checkRef{[&](const vfs::directory_entry &ent) noexcept -> llvm::Expected<bool> {
    tmp.clear();
    switch (ent.type()) {
    case fs::file_type::regular_file:
    case fs::file_type::directory_file:
      refFileT.try_emplace(ent.path().drop_front(refBaseLen), ent.type());
      break;
    default:
      break;
    }
    return true;
  }};
  if (llvm::Error err{iterDir<true>(tmp, checkRef, refVFS)}) {
    return err;
  }

  llvm::StringMap<fs::file_type> dirFileT;
  auto checkDir{[&](const vfs::directory_entry &ent) noexcept -> llvm::Expected<bool> {
    switch (ent.type()) {
    case fs::file_type::regular_file:
    case fs::file_type::directory_file:
      dirFileT.try_emplace(ent.path().drop_front(dirBaseLen), ent.type());
      break;
    default:
      break;
    }
    return true;
  }};
  if (llvm::Error err{iterDir<true>(dir, checkDir, dirVFS)}) {
    return err;
  }

  bool res{true};
  for (const auto &[relPath, fileT] : refFileT) {
    llvm::StringRef fileTStr{fileT == fs::file_type::regular_file ? "file" : "directory"};
    auto it{dirFileT.find(relPath)};
    if (it == dirFileT.end()) {
      logErr("Missing {} {}.", fileTStr, relPath);
      res = false;
      continue;
    }
    if (fileT != it->getValue()) {
      logErr("Expected {} to be a {}.", relPath, fileTStr);
      res = false;
    } else if (fileT == fs::file_type::regular_file) {
      llvm::Expected<bool> fileRes{cmpFiles(ref + pth::get_separator() + relPath,
                                            dir + pth::get_separator() + relPath, refVFS, dirVFS)};
      if (!fileRes) {
        return fileRes.takeError();
      }
      if (!*fileRes) {
        logErr("Mismatching file content for {}.", relPath);
        res = false;
      }
    }
    dirFileT.erase(it);
  }
  for (const auto &[relPath, fileT] : dirFileT) {
    llvm::StringRef fileTStr{fileT == fs::file_type::regular_file ? "file" : "directory"};
    logErr("Unexpected {} {}.", fileTStr, relPath);
    res = false;
  }
  return res;
}
