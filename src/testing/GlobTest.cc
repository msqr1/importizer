// Glob utilities testing. We trust that llvm::GlobPattern is already well-tested so we won't test
// creation and matching extensively.

#include "utils/Glob.hh"
#include "testing/MemFS.hh"
#include "testing/Unit.hh"
#include <array>
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

  INFO("Invalid glob");
  CHECK_UNARY_FALSE(invalid);

  std::optional<Glob> g{mkGlob("*")};
  if (!CHECK_UNARY(g)) {
    return;
  }
  std::optional<Glob> gNeg{mkGlob("!*")};
  if (!CHECK_UNARY(gNeg)) {
    return;
  }

  INFO("Negated match");
  CHECK_NE(g->match("hehe"), gNeg->match("hehe"));
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

  INFO("Empty dir path");
  CHECK_UNARY_FALSE(globDir("", rules, memFS));

  llvm::StringSet<> globbedCPPFilePaths;
  if (!CHECK_UNARY(mkGlobs(rules[0].globs, std::array<llvm::StringRef, 1>{"*.{hh,cc}"}))) {
    return;
  }
  rules[0].callback = [&globbedCPPFilePaths](const vfs::directory_entry &ent) -> void {
    if (ent.type() == fs::file_type::regular_file) {
      globbedCPPFilePaths.insert(ent.path());
    }
  };
  llvm::StringSet<> globbedCMakeFilePaths;
  if (!CHECK_UNARY(mkGlobs(rules[1].globs, std::array<llvm::StringRef, 1>{"*CMakeLists.txt"}))) {
    return;
  }
  rules[1].callback = [&globbedCMakeFilePaths](const vfs::directory_entry &ent) -> void {
    if (ent.type() == fs::file_type::regular_file) {
      globbedCMakeFilePaths.insert(ent.path());
    }
  };

  if (!CHECK_UNARY(globDir("/", rules, memFS))) {
    return;
  }

  INFO("C++ files");
  CHECK_EQ(cppFilePaths, globbedCPPFilePaths);

  INFO("CMake files");
  CHECK_EQ(cmakeFilePaths, globbedCMakeFilePaths);
}

TEST_SUITE_END();
