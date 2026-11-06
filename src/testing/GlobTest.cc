// Glob utilities tests. We trust that llvm::GlobPattern is already well-tested, so we won't test
// creation and matching too much.

#include "utils/Glob.hh"
#include "testing/Doctest.hh"
#include "testing/MemFS.hh"
#include "testing/Unit.hh"
#include <array>
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/StringSet.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/VirtualFileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <utility>

namespace fs = llvm::sys::fs;
namespace vfs = llvm::vfs;

TEST_SUITE_BEGIN("Glob");

TEST_CASE("Creation and negation") {
  SUBCASE("Invalid glob") { CHECK_UNARY_FALSE(ok(mkGlob("["))); }

  SUBCASE("Negated match") {
    llvm::Expected<Glob> g{mkGlob("*")};
    REQUIRE_UNARY(ok(g));

    llvm::Expected<Glob> gNeg{mkGlob("!*")};
    REQUIRE_UNARY(ok(gNeg));

    CHECK_NE(g->match("hehe"), gNeg->match("hehe"));
  }
}

constexpr std::array<llvm::StringRef, 2> cppFilesGlobs{{"*.hh", "*.cc"}};
constexpr std::array<llvm::StringRef, 1> cmakeFileGlobs{"*CMakeLists.txt"};
TEST_CASE("Glob directory") {
  vfs::InMemoryFileSystem memFS;

  SUBCASE("Empty paths") {
    std::array<GlobDirRule, 0> rules;
    CHECK_UNARY_FALSE(ok(globDir("", rules, memFS)));
  }

  SUBCASE("Real matching") {
    const llvm::StringSet<> cppFilePaths{"/lib/IO.hh", "/lib/IO.cc", "/src/Driver.cc"};
    const llvm::StringSet<> cmakeFilePaths{"/CMakeLists.txt", "/lib/CMakeLists.txt",
                                           "/src/CMakeLists.txt"};
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
      llvm::cantFail(addFile1(memFS, path));
    }
    std::array<GlobDirRule, 2> rules;

    llvm::StringSet<> globbedCPPFilePaths;
    llvm::Expected<GlobStorage> globs{mkGlobs(cppFilesGlobs)};
    REQUIRE_UNARY(ok(globs));
    rules[0].globs = std::move(*globs);
    rules[0].callback = [&](const vfs::directory_entry &ent) -> void {
      if (ent.type() == fs::file_type::regular_file) {
        // Without \ -> / conversion StringSet comparison will fail on Windows.
        globbedCPPFilePaths.insert(pth::convert_to_slash(ent.path()));
      }
    };

    llvm::StringSet<> globbedCMakeFilePaths;
    globs = mkGlobs(cmakeFileGlobs);
    REQUIRE_UNARY(ok(globs));
    rules[1].globs = std::move(*globs);
    rules[1].callback = [&](const vfs::directory_entry &ent) -> void {
      if (ent.type() == fs::file_type::regular_file) {
        // Without \ -> / conversion StringSet comparison will fail on Windows.
        globbedCMakeFilePaths.insert(pth::convert_to_slash(ent.path()));
      }
    };
    REQUIRE_UNARY(ok(globDir("/", rules, memFS)));
    CHECK_EQ(cppFilePaths, globbedCPPFilePaths);
    CHECK_EQ(cmakeFilePaths, globbedCMakeFilePaths);
  }
}

TEST_SUITE_END();
