// Glob utilities tests. We trust that llvm::GlobPattern is already well-tested, so we won't test
// creation and matching too much.

#include "utils/Glob.hh"
#include "testing/MemFS.hh"
#include "testing/Unit.hh"
#include <array>
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/StringSet.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/VirtualFileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <utility>

namespace fs = llvm::sys::fs;
namespace vfs = llvm::vfs;

TEST_SUITE_BEGIN("Glob");

TEST_CASE("Creation and negation") {
  SUBCASE("Invalid glob") {
    llvm::Expected<Glob> invalid{mkGlob("[]")};
    CHECK_UNARY_FALSE(!!invalid);
  }

  SUBCASE("Negated match") {
    llvm::Expected<Glob> g{mkGlob("*")};
    if (!CHECK_UNARY(!!g)) {
      return;
    }
    llvm::Expected<Glob> gNeg{mkGlob("!*")};
    if (!CHECK_UNARY(!!gNeg)) {
      return;
    }
    CHECK_NE(g->match("hehe"), gNeg->match("hehe"));
  }
}

constexpr std::array<llvm::StringRef, 2> cppFilesGlobs{{"*.hh", "*.cc"}};
constexpr std::array<llvm::StringRef, 1> cmakeFileGlobs{"*CMakeLists.txt"};
TEST_CASE("Glob directory") {
  SUBCASE("Empty paths") {
    vfs::InMemoryFileSystem memFS;
    std::array<GlobDirRule, 1> rules;
    CHECK_UNARY(!!globDir("", rules, memFS));
  }

  SUBCASE("Real matching") {
    const llvm::StringSet<> cppFilePaths{"/lib/IO.hh", "/lib/IO.cc", "/src/Driver.cc"};
    const llvm::StringSet<> cmakeFilePaths{"/CMakeLists.txt", "/lib/CMakeLists.txt",
                                           "/src/CMakeLists.txt"};
    vfs::InMemoryFileSystem memFS;
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
      addFile1(memFS, path);
    }
    std::array<GlobDirRule, 2> rules;

    llvm::StringSet<> globbedCPPFilePaths;
    llvm::Expected<GlobStorage> globs{mkGlobs(cppFilesGlobs)};
    if (!CHECK_UNARY(!!globs)) {
      return;
    }
    rules[0].globs = std::move(*globs);
    rules[0].callback = [&](const vfs::directory_entry &ent) -> void {
      if (ent.type() == fs::file_type::regular_file) {
        // Without \ -> / conversion StringSet comparison will fail on Windows.
        globbedCPPFilePaths.insert(pth::convert_to_slash(ent.path()));
      }
    };
    llvm::StringSet<> globbedCMakeFilePaths;
    globs = mkGlobs(cmakeFileGlobs);
    if (!CHECK_UNARY(!!globs)) {
      return;
    }
    rules[1].globs = std::move(*globs);
    rules[1].callback = [&](const vfs::directory_entry &ent) -> void {
      if (ent.type() == fs::file_type::regular_file) {
        // Without \ -> / conversion StringSet comparison will fail on Windows.
        globbedCMakeFilePaths.insert(pth::convert_to_slash(ent.path()));
      }
    };

    if (!CHECK_UNARY_FALSE(!!globDir("/", rules, memFS))) {
      return;
    }
    CHECK_EQ(cppFilePaths, globbedCPPFilePaths);
    CHECK_EQ(cmakeFilePaths, globbedCMakeFilePaths);
  }
}

TEST_SUITE_END();
