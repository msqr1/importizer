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
#include <utility>

namespace pth = llvm::sys::path;
namespace fs = llvm::sys::fs;
namespace vfs = llvm::vfs;

namespace {

auto basePathLen(const llvm::Twine &path) noexcept -> llvm::Expected<size_t> {
  llvm::SmallString<64> tmp;
  if (path.isTriviallyEmpty() || (path.toVector(tmp), tmp.empty())) {
    return llvm::createStringError("path is empty");
  }
  return tmp.size() + static_cast<size_t>(!pth::is_separator(tmp.back()));
}

auto buildFileMap(const llvm::Twine &dir, vfs::FileSystem &vfs, size_t baseLen) noexcept
    -> llvm::Expected<llvm::StringMap<fs::file_type>> {
  llvm::StringMap<fs::file_type> fileMap{};

  auto checkDir{[&](const vfs::directory_entry &ent) noexcept -> llvm::Expected<bool> {
    if (ent.type() == fs::file_type::regular_file || ent.type() == fs::file_type::directory_file) {
      fileMap.try_emplace(ent.path().drop_front(baseLen), ent.type());
    }
    return true;
  }};

  if (llvm::Error err{iterDir<true>(dir, checkDir, vfs)}) {
    return std::move(err);
  }

  return fileMap;
}

} // namespace

auto realFS() noexcept -> vfs::FileSystem & {
  static std::unique_ptr<vfs::FileSystem> realFS{vfs::createPhysicalFileSystem()};
  return *realFS;
}

auto cpyDir(const llvm::Twine &src, const llvm::Twine &dst, vfs::FileSystem &srcVFS) noexcept
    -> llvm::Error {
  auto dstBaseLen{basePathLen(dst)};
  if (!dstBaseLen) {
    return llvm::createStringError(
        llvm::formatv("When copying directory: {}.", llvm::toString(dstBaseLen.takeError())));
  }

  auto srcBaseLen{basePathLen(src)};
  if (!srcBaseLen) {
    return llvm::createStringError(
        llvm::formatv("When copying directory: {}.", llvm::toString(srcBaseLen.takeError())));
  }

  if (std::error_code err{fs::create_directories(dst)}) {
    return llvm::createStringError(
        err, llvm::formatv("When creating directory {}: {}.", dst, err.message()));
  }

  llvm::SmallString<64> tmpDst{};
  dst.toVector(tmpDst);

  auto copy{[&](const vfs::directory_entry &ent) noexcept -> llvm::Expected<bool> {
    llvm::StringRef relPath{ent.path().drop_front(*srcBaseLen)};
    llvm::SmallString<128> tmp{};
    pth::append(tmp, tmpDst, relPath);

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
    default:
      break;
    }
    return true;
  }};

  llvm::SmallString<64> tmpSrc{};
  src.toVector(tmpSrc);
  return iterDir<true>(tmpSrc, copy, srcVFS);
}

auto cmpFiles(const llvm::Twine &file1, const llvm::Twine &file2, vfs::FileSystem &file1VFS,
              vfs::FileSystem &file2VFS) noexcept -> llvm::Expected<bool> {
  const auto buf1{file1VFS.getBufferForFile(file1, -1, false, false, false)};
  if (std::error_code err{buf1.getError()}) {
    return llvm::createStringError(err,
                                   llvm::formatv("When opening {}: {}.", file1, err.message()));
  }
  const auto buf2{file2VFS.getBufferForFile(file2, -1, false, false, false)};
  if (std::error_code err{buf2.getError()}) {
    return llvm::createStringError(err,
                                   llvm::formatv("When opening {}: {}.", file2, err.message()));
  }
  return (**buf1).getBuffer() == (**buf2).getBuffer();
}

auto cmpDirs(const llvm::Twine &dir, const llvm::Twine &ref, vfs::FileSystem &dirVFS,
             vfs::FileSystem &refVFS) noexcept -> llvm::Expected<bool> {
  auto dirBaseLen{basePathLen(dir)};
  if (!dirBaseLen) {
    return llvm::createStringError(
        llvm::formatv("When comparing directories: {}.", llvm::toString(dirBaseLen.takeError())));
  }

  auto refBaseLen{basePathLen(ref)};
  if (!refBaseLen) {
    return llvm::createStringError(
        llvm::formatv("When comparing directories: {}.", llvm::toString(refBaseLen.takeError())));
  }

  llvm::SmallString<64> tmpRef{};
  ref.toVector(tmpRef);
  auto refFileT{buildFileMap(tmpRef, refVFS, *refBaseLen)};
  if (!refFileT) {
    return refFileT.takeError();
  }

  llvm::SmallString<64> tmpDir{};
  dir.toVector(tmpDir);
  auto dirFileT{buildFileMap(tmpDir, dirVFS, *dirBaseLen)};
  if (!dirFileT) {
    return dirFileT.takeError();
  }

  bool res{true};
  for (const auto &[relPath, fileT] : *refFileT) {
    llvm::StringRef fileTStr{fileT == fs::file_type::regular_file ? "file" : "directory"};
    auto it{dirFileT->find(relPath)};

    if (it == dirFileT->end()) {
      logErr("When comparing directories: Missing {} {}.", fileTStr, relPath);
      res = false;
      continue;
    }

    if (fileT != it->getValue()) {
      logErr("When comparing directories: Expected {} to be a {}.", relPath, fileTStr);
      res = false;
    } else if (fileT == fs::file_type::regular_file) {
      llvm::Expected<bool> fileRes{cmpFiles(ref + pth::get_separator() + relPath,
                                            dir + pth::get_separator() + relPath, refVFS, dirVFS)};
      if (!fileRes) {
        return fileRes.takeError();
      }
      if (!*fileRes) {
        logErr("When comparing directories: Mismatching file content for {}.", relPath);
        res = false;
      }
    }
    dirFileT->erase(it);
  }

  for (const auto &[relPath, fileT] : *dirFileT) {
    llvm::StringRef fileTStr{fileT == fs::file_type::regular_file ? "file" : "directory"};
    logErr("When comparing directories: Unexpected {} {}.", fileTStr, relPath);
    res = false;
  }

  return res;
}
