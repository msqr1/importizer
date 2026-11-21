// Filesystem utilities tests.

#include "testing/Doctest.hh"
#include "testing/MemFS.hh"
#include "utils/FS.hh"
#include <llvm/Support/Error.h>
#include <llvm/Support/VirtualFileSystem.h>
#include <llvm/Support/raw_ostream.h>

SUITE("FS");

CASE("Compare files") {
  vfs::InMemoryFileSystem memFS;

  SUBCASE("Empty paths") {
    CHECK_FALSE(cmpFiles("", "/", memFS, memFS));
    CHECK_FALSE(cmpFiles("/", "", memFS, memFS));
  }

  SUBCASE("Same content") {
    llvm::cantFail(addFile1(memFS, "/File.txt"));
    llvm::cantFail(addFile1(memFS, "/Meow.txt"));

    llvm::Expected<bool> cmpRes{cmpFiles("/File.txt", "/Meow.txt", memFS, memFS)};
    REQUIRE(cmpRes);
    CHECK(*cmpRes);
  }

  SUBCASE("Mismatching content") {
    llvm::cantFail(addFile1(memFS, "/File.txt"));
    llvm::cantFail(addFile2(memFS, "/Meow.txt"));

    llvm::Expected<bool> cmpRes{cmpFiles("/File.txt", "/Meow.txt", memFS, memFS)};
    REQUIRE(cmpRes);
    CHECK_FALSE(*cmpRes);
  }
}

CASE("Compare directories") {
  vfs::InMemoryFileSystem memFS;

  SUBCASE("Empty paths") {
    CHECK_FALSE(cmpDirs("", "/", memFS, memFS));
    CHECK_FALSE(cmpDirs("/", "", memFS, memFS));
  }

  SUBCASE("Identical directories") {
    llvm::cantFail(addFile1(memFS, "/ref/File.txt"));
    llvm::cantFail(addFile1(memFS, "/dir/File.txt"));

    llvm::Expected<bool> cmpRes{cmpDirs("/dir", "/ref", memFS, memFS)};
    REQUIRE(cmpRes);
    CHECK(*cmpRes);
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
    REQUIRE(cmpRes);
    CHECK_FALSE(*cmpRes);
  }
}

END_SUITE();
