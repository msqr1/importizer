#include "testing/MemFS.hh"
#include "utils/FS.hh"
#include <llvm/ADT/Twine.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/FormatVariadic.h>
#include <llvm/Support/VirtualFileSystem.h>
#include <optional>

namespace {

auto const BUF1{llvm::MemoryBuffer::getMemBuffer("When I say six, what do you say?")};
auto const BUF2{llvm::MemoryBuffer::getMemBuffer("Seven! Get it? Cuz, cuz 67!")};

} // namespace

auto addDir(vfs::InMemoryFileSystem &memFS, llvm::Twine const &path) -> llvm::Error {
  if (memFS.addFileNoOwn(path, 0, *BUF1, std::nullopt, std::nullopt,
                         llvm::sys::fs::file_type::directory_file, std::nullopt)) {
    return llvm::Error::success();
  }
  return llvm::createStringError(
      llvm::formatv("Directory {} exists with different content.", path));
}

auto addFile1(vfs::InMemoryFileSystem &memFS, llvm::Twine const &path) -> llvm::Error {
  if (memFS.addFileNoOwn(path, 0, *BUF1)) {
    return llvm::Error::success();
  }
  return llvm::createStringError(llvm::formatv("File {} exists with different content.", path));
}

auto addFile2(vfs::InMemoryFileSystem &memFS, llvm::Twine const &path) -> llvm::Error {
  if (memFS.addFileNoOwn(path, 0, *BUF2)) {
    return llvm::Error::success();
  }
  return llvm::createStringError(llvm::formatv("File {} exists with different content.", path));
}
