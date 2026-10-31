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
  InitableMemFS memFS{[](vfs::InMemoryFileSystem &FS) -> void {
    /*
      /
      ├── File.txt
      ├── Meow.txt
      └── Woof.txt
    */
    addFile1(FS, "/File.txt");
    addFile1(FS, "/Meow.txt");
    addFile2(FS, "/Woof.txt");
  }};

  INFO("Empty file1 path");
  CHECK_EQ(cmpFiles("", "/", memFS, memFS), CmpRes::Err);

  INFO("Empty file2 path");
  CHECK_EQ(cmpFiles("/", "", memFS, memFS), CmpRes::Err);

  INFO("Same content");
  CHECK_EQ(cmpFiles("/File.txt", "/Meow.txt", memFS, memFS), CmpRes::Same);

  INFO("Mismatched content");
  CHECK_EQ(cmpFiles("/File.txt", "/Woof.txt", memFS, memFS), CmpRes::Mismatch);
}

TEST_CASE("Compare directories") {
  InitableMemFS memFS{[](vfs::InMemoryFileSystem &FS) -> void {
    /*
      /
      ├── ref/
      │   └── File.txt
      ├── dir1/
      │   └── File.txt
      ├── dir2/
      ├── dir3/
      │   └── File.txt
      ├── dir4/
      │   ├── File.txt
      │   └── Meow.txt
      └── dir5/
          └── File.txt
    */
    addFile1(FS, "/ref/File.txt");
    addFile1(FS, "/dir1/File.txt");
    addDir(FS, "/dir2");
    addFile2(FS, "/dir3/File.txt");
    addFile1(FS, "/dir4/File.txt");
    addFile1(FS, "/dir4/Meow.txt");
    addDir(FS, "/dir5/File.txt");
  }};

  INFO("Empty dir path");
  CHECK_EQ(cmpDirs("", "/", memFS, memFS), CmpRes::Err);

  INFO("Empty ref path");
  CHECK_EQ(cmpDirs("/", "", memFS, memFS), CmpRes::Err);

  INFO("Identical directories");
  CHECK_EQ(cmpDirs("/dir1", "/ref", memFS, memFS), CmpRes::Same);

  INFO("Missing file in target");
  CHECK_EQ(cmpDirs("/dir2", "/ref", memFS, memFS), CmpRes::Mismatch);

  INFO("Mismatched file content");
  CHECK_EQ(cmpDirs("/dir3", "/ref", memFS, memFS), CmpRes::Mismatch);

  INFO("Unexpected extra file");
  CHECK_EQ(cmpDirs("/dir4", "/ref", memFS, memFS), CmpRes::Mismatch);

  INFO("Unexpected file type");
  CHECK_EQ(cmpDirs("/dir5", "/ref", memFS, memFS), CmpRes::Mismatch);
}

TEST_SUITE_END();
