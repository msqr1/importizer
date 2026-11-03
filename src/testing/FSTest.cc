// Filesystem utilities test

#include "utils/FS.hh"
#include "testing/MemFS.hh"
#include "testing/Unit.hh"
#include <llvm/ADT/STLFunctionalExtras.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/MemoryBufferRef.h>
#include <llvm/Support/VirtualFileSystem.h>
#include <llvm/Support/raw_ostream.h>

TEST_SUITE_BEGIN("FS");

TEST_CASE("Compare files") {
  SUBCASE("Empty paths") {
    vfs::InMemoryFileSystem memFS;
    CHECK_UNARY_FALSE(cmpFiles("", "/", memFS, memFS));
    CHECK_UNARY_FALSE(cmpFiles("/", "", memFS, memFS));
  }
  SUBCASE("Same content") {
    vfs::InMemoryFileSystem memFS;
    addFile1(memFS, "/File.txt");
    addFile1(memFS, "/Meow.txt");
    llvm::Expected<bool> cmpRes{cmpFiles("/File.txt", "/Meow.txt", memFS, memFS)};
    if (!CHECK_UNARY(!!cmpRes)) {
      return;
    }
    CHECK_UNARY(*cmpRes);
  }
  SUBCASE("Mismatched content") {
    vfs::InMemoryFileSystem memFS;
    addFile1(memFS, "/File.txt");
    addFile2(memFS, "/Meow.txt");
    llvm::Expected<bool> cmpRes{cmpFiles("/File.txt", "/Meow.txt", memFS, memFS)};
    if (!CHECK_UNARY(!!cmpRes)) {
      return;
    }
    CHECK_UNARY_FALSE(*cmpRes);
  }
}
TEST_CASE("Compare directories") {
  SUBCASE("Empty paths") {
    vfs::InMemoryFileSystem memFS;
    CHECK_UNARY_FALSE(cmpDirs("", "/", memFS, memFS));
    CHECK_UNARY_FALSE(cmpDirs("/", "", memFS, memFS));
  }
  SUBCASE("Identical directories") {
    vfs::InMemoryFileSystem memFS;
    addFile1(memFS, "/ref/File.txt");
    addFile1(memFS, "/dir/File.txt");
    llvm::Expected<bool> cmpRes{cmpDirs("/dir", "/ref", memFS, memFS)};
    if (!CHECK_UNARY(!!cmpRes)) {
      return;
    }
    CHECK_UNARY(*cmpRes);
  }
  SUBCASE("Missing file") {
    vfs::InMemoryFileSystem memFS;
    addFile1(memFS, "/ref/File.txt");
    addDir(memFS, "/dir");
    llvm::Expected<bool> cmpRes{cmpDirs("/dir", "/ref", memFS, memFS)};
    if (!CHECK_UNARY(!!cmpRes)) {
      return;
    }
    CHECK_UNARY_FALSE(*cmpRes);
  }
  SUBCASE("Mismatched file content") {
    vfs::InMemoryFileSystem memFS;
    addFile1(memFS, "/ref/File.txt");
    addFile2(memFS, "/dir/File.txt");
    llvm::Expected<bool> cmpRes{cmpDirs("/dir", "/ref", memFS, memFS)};
    if (!CHECK_UNARY(!!cmpRes)) {
      return;
    }
    CHECK_UNARY_FALSE(*cmpRes);
  }
  SUBCASE("Unexpected extra file") {
    vfs::InMemoryFileSystem memFS;
    addFile1(memFS, "/ref/File.txt");
    addFile1(memFS, "/dir/File.txt");
    addFile1(memFS, "/dir/Meow.txt");
    llvm::Expected<bool> cmpRes{cmpDirs("/dir", "/ref", memFS, memFS)};
    if (!CHECK_UNARY(!!cmpRes)) {
      return;
    }
    CHECK_UNARY_FALSE(*cmpRes);
  }
  SUBCASE("Unexpected file type") {
    vfs::InMemoryFileSystem memFS;
    addFile1(memFS, "/ref/File.txt");
    addDir(memFS, "/dir/File.txt");
    llvm::Expected<bool> cmpRes{cmpDirs("/dir", "/ref", memFS, memFS)};
    if (!CHECK_UNARY(!!cmpRes)) {
      return;
    }
    CHECK_UNARY_FALSE(*cmpRes);
  }
}

TEST_SUITE_END();
