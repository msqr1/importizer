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
    llvm::Expected<bool> res{cmpFiles("/File.txt", "/Meow.txt", memFS, memFS)};
    CHECK_UNARY(res);
    CHECK_UNARY(*res);
  }
  SUBCASE("Mismatched content") {
    vfs::InMemoryFileSystem memFS;
    addFile1(memFS, "/File.txt");
    addFile2(memFS, "/Meow.txt");
    llvm::Expected<bool> res{cmpFiles("/File.txt", "/Meow.txt", memFS, memFS)};
    CHECK_UNARY(res);
    CHECK_UNARY_FALSE(*res);
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
    llvm::Expected<bool> res{cmpDirs("/dir", "/ref", memFS, memFS)};
    CHECK_UNARY(res);
    CHECK_UNARY(*res);
  }
  SUBCASE("Missing file") {
    vfs::InMemoryFileSystem memFS;
    addFile1(memFS, "/ref/File.txt");
    addDir(memFS, "/dir");
    CHECK_EQ(cmpDirs("/dir", "/ref", memFS, memFS), CmpRes::Mismatch);
  }
  SUBCASE("Mismatched file content") {
    vfs::InMemoryFileSystem memFS;
    addFile1(memFS, "/ref/File.txt");
    addFile2(memFS, "/dir/File.txt");
    CHECK_EQ(cmpDirs("/dir", "/ref", memFS, memFS), CmpRes::Mismatch);
  }
  SUBCASE("Unexpected extra file") {
    vfs::InMemoryFileSystem memFS;
    addFile1(memFS, "/ref/File.txt");
    addFile1(memFS, "/dir/File.txt");
    addFile1(memFS, "/dir/Meow.txt");
    CHECK_EQ(cmpDirs("/dir", "/ref", memFS, memFS), CmpRes::Mismatch);
  }
  SUBCASE("Unexpected file type") {
    vfs::InMemoryFileSystem memFS;
    addFile1(memFS, "/ref/File.txt");
    addDir(memFS, "/dir/File.txt");
    CHECK_EQ(cmpDirs("/dir", "/ref", memFS, memFS), CmpRes::Mismatch);
  }
}

TEST_SUITE_END();
