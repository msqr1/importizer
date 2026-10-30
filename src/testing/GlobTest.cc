// Glob utilities testing. We trust that llvm::GlobPattern is already well-tested so we won't test
// creation and matching extensively.

#include "utils/Glob.hh"
#include "testing/MemFS.hh"
#include <array>
#include <doctest/doctest.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/StringSet.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/VirtualFileSystem.h>
#include <optional>

namespace fs = llvm::sys::fs;
namespace vfs = llvm::vfs;

TEST_SUITE_BEGIN("Glob");

TEST_CASE("Creation and negation") {
  std::optional<Glob> invalid{mkGlob("[]")};
  CHECK_FALSE(invalid);

  std::optional<Glob> g{mkGlob("*")};
  CHECK(g);

  std::optional<Glob> gInv{mkGlob("!*")};
  CHECK(gInv);

  CHECK_NE(g->match("hehe"), gInv->match("hehe"));
}

TEST_CASE("Glob directory") {
  llvm::StringSet<> cppFilePaths{"/lib/IO.hh", "/lib/IO.cc", "/src/Driver.cc"};
  llvm::StringSet<> cmakeFilePaths{"/CMakeLists.txt", "/lib/CMakeLists.txt", "/src/CMakeLists.txt"};
  InitableMemFS memFS{[&](vfs::InMemoryFileSystem &FS) -> void {
    /*
      /
      ├── CMakeLists.txt
      ├── lib/
      │   ├── CMakeLists.txt
      │   ├── IO.hh
      │   └── IO.cc
      └── src/
          ├── CMakeLists.txt
          └── Driver.cc
    */
    for (llvm::StringRef path :
         llvm::concat<llvm::StringRef>(cppFilePaths.keys(), cmakeFilePaths.keys())) {
      addFile1(FS, path);
    }
  }};
  std::array<GlobDirRule, 2> rules;

  llvm::StringSet<> globbedCPPFilePaths;
  CHECK(mkGlobs(rules[0].globs, std::array<llvm::StringRef, 1>{"*.{hh,cc}"}));
  rules[0].callback = [&globbedCPPFilePaths](const vfs::directory_entry &ent) -> void {
    if (ent.type() == fs::file_type::regular_file) {
      globbedCPPFilePaths.insert(ent.path());
    }
  };

  llvm::StringSet<> globbedCMakeFilePaths;
  CHECK(mkGlobs(rules[1].globs, std::array<llvm::StringRef, 1>{"*CMakeLists.txt"}));
  rules[1].callback = [&globbedCMakeFilePaths](const vfs::directory_entry &ent) -> void {
    if (ent.type() == fs::file_type::regular_file) {
      globbedCMakeFilePaths.insert(ent.path());
    }
  };

  CHECK(globDir("/", rules, memFS));
  CHECK_EQ(cppFilePaths, globbedCPPFilePaths);
  CHECK_EQ(cmakeFilePaths, globbedCMakeFilePaths);
}

TEST_SUITE_END();
