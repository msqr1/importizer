// Filesystem utilities tests.

#include "utils/FS.hh"
#include "testing/Doctest.hh"
#include "testing/MemFS.hh"
#include "testing/Unit.hh"
#include <llvm/ADT/STLFunctionalExtras.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/MemoryBufferRef.h>
#include <llvm/Support/VirtualFileSystem.h>
#include <llvm/Support/raw_ostream.h>

TEST_SUITE_BEGIN("FS");

TEST_CASE("Compare files") {
  vfs::InMemoryFileSystem memFS;

  SUBCASE("Empty paths") {
    CHECK_UNARY_FALSE(ok(cmpFiles("", "/", memFS, memFS)));
    CHECK_UNARY_FALSE(ok(cmpFiles("/", "", memFS, memFS)));
  }

  SUBCASE("Same content") {
    llvm::cantFail(addFile1(memFS, "/File.txt"));
    llvm::cantFail(addFile1(memFS, "/Meow.txt"));

    llvm::Expected<bool> cmpRes{cmpFiles("/File.txt", "/Meow.txt", memFS, memFS)};
    REQUIRE_UNARY(ok(cmpRes));
    CHECK_UNARY(*cmpRes);
  }

  SUBCASE("Mismatching content") {
    llvm::cantFail(addFile1(memFS, "/File.txt"));
    llvm::cantFail(addFile2(memFS, "/Meow.txt"));

    llvm::Expected<bool> cmpRes{cmpFiles("/File.txt", "/Meow.txt", memFS, memFS)};
    REQUIRE_UNARY(ok(cmpRes));
    CHECK_UNARY_FALSE(*cmpRes);
  }
}

TEST_CASE("Compare directories") {
  vfs::InMemoryFileSystem memFS;

  SUBCASE("Empty paths") {
    CHECK_UNARY_FALSE(ok(cmpDirs("", "/", memFS, memFS)));
    CHECK_UNARY_FALSE(ok(cmpDirs("/", "", memFS, memFS)));
  }

  SUBCASE("Identical directories") {
    llvm::cantFail(addFile1(memFS, "/ref/File.txt"));
    llvm::cantFail(addFile1(memFS, "/dir/File.txt"));

    llvm::Expected<bool> cmpRes{cmpDirs("/dir", "/ref", memFS, memFS)};
    REQUIRE_UNARY(ok(cmpRes));
    CHECK_UNARY(*cmpRes);
  }

  SUBCASE("Mismatching directories") {
    SUBCASE("Missing file") {
      llvm::cantFail(addFile1(memFS, "/ref/File.txt"));
      llvm::cantFail(addDir(memFS, "/dir"));
    }

    SUBCASE("Mismatching file content") {
      llvm::cantFail(addFile1(memFS, "/ref/File.txt"));
      llvm::cantFail(addFile2(memFS, "/dir/File.txt"));
    }

    SUBCASE("Unexpected extra file") {
      llvm::cantFail(addFile1(memFS, "/ref/File.txt"));
      llvm::cantFail(addFile1(memFS, "/dir/File.txt"));
      llvm::cantFail(addFile1(memFS, "/dir/Meow.txt"));
    }

    SUBCASE("Unexpected file type") {
      llvm::cantFail(addFile1(memFS, "/ref/File.txt"));
      llvm::cantFail(addDir(memFS, "/dir/File.txt"));
    }

    llvm::Expected<bool> cmpRes{cmpDirs("/dir", "/ref", memFS, memFS)};
    REQUIRE_UNARY(ok(cmpRes));
    CHECK_UNARY_FALSE(*cmpRes);
  }
}

TEST_SUITE_END();
