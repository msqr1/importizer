#include "testing/MemFS.hh"
#include "utils/FS.hh"
#include <llvm/ADT/STLFunctionalExtras.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Twine.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/FormatVariadic.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/MemoryBufferRef.h>
#include <llvm/Support/VirtualFileSystem.h>
#include <optional>

namespace {

const auto BUF1{llvm::MemoryBuffer::getMemBuffer("When I say six, what do you say?")};
const auto BUF2{llvm::MemoryBuffer::getMemBuffer("Seven! Get it? Cuz, cuz 67!")};

} // namespace

auto addDir(vfs::InMemoryFileSystem &memFS, const llvm::Twine &path) -> llvm::Error {
  bool res{memFS.addFileNoOwn(path, 0, *BUF1, std::nullopt, std::nullopt,
                              llvm::sys::fs::file_type::directory_file, std::nullopt)};
  if (res) {
    return llvm::Error::success();
  }
  return llvm::createStringError(
      llvm::formatv("Directory {} exists with different content.", path));
}

auto addFile1(vfs::InMemoryFileSystem &memFS, const llvm::Twine &path) -> llvm::Error {
  bool res{memFS.addFileNoOwn(path, 0, *BUF1)};
  if (res) {
    return llvm::Error::success();
  }
  return llvm::createStringError(llvm::formatv("File {} exists with different content.", path));
}

auto addFile2(vfs::InMemoryFileSystem &memFS, const llvm::Twine &path) -> llvm::Error {
  bool res{memFS.addFileNoOwn(path, 0, *BUF2)};
  if (res) {
    return llvm::Error::success();
  }
  return llvm::createStringError(llvm::formatv("File {} exists with different content.", path));
}
