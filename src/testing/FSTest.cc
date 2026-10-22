#include "utils/FS.hh"
#include <doctest/doctest.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/MemoryBufferRef.h>
#include <llvm/Support/VirtualFileSystem.h>

namespace vfs = llvm::vfs;

namespace {
const llvm::StringRef content1{"When I say six, what do you say?"};
const llvm::StringRef content2{"Seven!"};
} // namespace

TEST_SUITE_BEGIN("FS");

TEST_CASE("Compare files") {
  const auto buf1{llvm::MemoryBuffer::getMemBuffer(content1)};
  const auto buf2{llvm::MemoryBuffer::getMemBuffer(content2)};
  vfs::InMemoryFileSystem memFS;
  /*
    /
    ├── file1.txt - Same content as file2.txt
    ├── file2.txt
    └── file3.txt - Different content
  */
  memFS.addFileNoOwn("/file1.txt", 0L, *buf1);
  memFS.addFileNoOwn("/file2.txt", 0L, *buf1);
  memFS.addFileNoOwn("/file3.txt", 0L, *buf2);

  SUBCASE("Empty file1 path") {
    CHECK(cmpFiles("", "/file1.txt", memFS, memFS) == CmpRes::Err);
  }
  SUBCASE("Empty file2 path") {
    CHECK(cmpFiles("/file1.txt", "", memFS, memFS) == CmpRes::Err);
  }
  SUBCASE("Same content") {
    CHECK(cmpFiles("/file1.txt", "/file2.txt", memFS, memFS) == CmpRes::Same);
  }
  SUBCASE("Different content") {
    CHECK(cmpFiles("/file1.txt", "/file3.txt", memFS, memFS) ==
          CmpRes::Mismatch);
  }
}

TEST_CASE("Compare directories") {
  vfs::InMemoryFileSystem memFS;

  SUBCASE("Empty dir path") {
    CHECK(cmpDirs("", "/dir1", memFS, memFS) == CmpRes::Err);
  }
  SUBCASE("Empty ref path") {
    CHECK(cmpDirs("/dir2", "", memFS, memFS) == CmpRes::Err);
  }
}

TEST_SUITE_END();
