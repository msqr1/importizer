#include "testing/MemFS.hh"
#include "utils/FS.hh"
#include <llvm/ADT/STLFunctionalExtras.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Twine.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/MemoryBufferRef.h>
#include <llvm/Support/VirtualFileSystem.h>
#include <optional>

namespace {

const auto buf1{llvm::MemoryBuffer::getMemBuffer("When I say six, what do you say?")};
const auto buf2{llvm::MemoryBuffer::getMemBuffer("Seven! Get it? Cuz, cuz 67!")};

} // namespace

auto addDir(vfs::InMemoryFileSystem &memFS, const llvm::Twine &path) -> bool {
  return memFS.addFileNoOwn(path, 0, *buf1, std::nullopt, std::nullopt,
                            llvm::sys::fs::file_type::directory_file, std::nullopt);
}

auto addFile1(vfs::InMemoryFileSystem &memFS, const llvm::Twine &path) -> bool {
  return memFS.addFileNoOwn(path, 0, *buf1);
}

auto addFile2(vfs::InMemoryFileSystem &memFS, const llvm::Twine &path) -> bool {
  return memFS.addFileNoOwn(path, 0, *buf2);
}

InitableMemFS::InitableMemFS(llvm::function_ref<void(vfs::InMemoryFileSystem &memFS)> initFn) {
  initFn(*this);
}
