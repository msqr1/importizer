#include "utils/FS.hh"
#include <doctest/doctest.h>
#include <llvm/ADT/STLFunctionalExtras.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/MemoryBufferRef.h>
#include <llvm/Support/VirtualFileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <optional>

namespace vfs = llvm::vfs;
namespace fs = llvm::sys::fs;

namespace {

const auto buf1{llvm::MemoryBuffer::getMemBuffer("When I say six, what do you say?")};
const auto buf2{llvm::MemoryBuffer::getMemBuffer("Seven!")};
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

class InitableMemFS : public vfs::InMemoryFileSystem {
public:
  InitableMemFS(llvm::function_ref<void(vfs::InMemoryFileSystem &memFS)> initFn) { initFn(*this); }
};

} // namespace

TEST_SUITE_BEGIN("FS");

TEST_CASE("Compare files") {
  static InitableMemFS memFS{[](vfs::InMemoryFileSystem &FS) -> void {
    /*
      /
      ├── file.txt
      ├── meow.txt - Same content as file1.txt
      └── woof.txt - Different content from file1.txt
    */
    addFile1(FS, "/file.txt");
    addFile1(FS, "/meow.txt");
    addFile2(FS, "/woof.txt");
  }};
  SUBCASE("Empty file1 path") { CHECK(cmpFiles("", "/", memFS, memFS) == CmpRes::Err); }
  SUBCASE("Empty file2 path") { CHECK(cmpFiles("/", "", memFS, memFS) == CmpRes::Err); }
  SUBCASE("Same content") {
    CHECK(cmpFiles("/file.txt", "/meow.txt", memFS, memFS) == CmpRes::Same);
  }
  SUBCASE("Different content") {
    CHECK(cmpFiles("/file.txt", "/woof.txt", memFS, memFS) == CmpRes::Mismatch);
  }
}

TEST_CASE("Compare directories") {
  static InitableMemFS memFS{[](vfs::InMemoryFileSystem &FS) -> void {
    /*
      /
      ├── ref/
      │   └── file.txt
      ├── dir1/         - Same as ref/
      │   └── file.txt
      ├── dir2/         - Missing file
      ├── dir3/
      │   └── file.txt  - Mismatched file content
      ├── dir4/
      │   ├── file.txt
      │   └── meow.txt  - Unexpected file
      └── dir5/
          └── file.txt - Unexpected file type
    */
    addFile1(FS, "/ref/file.txt");
    addFile1(FS, "/dir1/file.txt");
    addDir(FS, "/dir2");
    addFile2(FS, "/dir3/file.txt");
    addFile1(FS, "/dir4/file.txt");
    addFile1(FS, "/dir4/meow.txt");
    addDir(FS, "/dir5/file.txt");
  }};

  SUBCASE("Empty dir path") { CHECK(cmpDirs("", "/", memFS, memFS) == CmpRes::Err); }
  SUBCASE("Empty ref path") { CHECK(cmpDirs("/", "", memFS, memFS) == CmpRes::Err); }
  SUBCASE("Same") { CHECK(cmpDirs("/dir1", "/ref", memFS, memFS) == CmpRes::Same); }
  SUBCASE("Missing file") { CHECK(cmpDirs("/dir2", "/ref", memFS, memFS) == CmpRes::Mismatch); }
  SUBCASE("Mismatched file content") {
    CHECK(cmpDirs("/dir3", "/ref", memFS, memFS) == CmpRes::Mismatch);
  }
  SUBCASE("Unexpected file") { CHECK(cmpDirs("/dir4", "/ref", memFS, memFS) == CmpRes::Mismatch); }
  SUBCASE("Unexpected file type") {
    CHECK(cmpDirs("/dir5", "/ref", memFS, memFS) == CmpRes::Mismatch);
  }
}

TEST_CASE("Copy directory") {}

TEST_SUITE_END();
