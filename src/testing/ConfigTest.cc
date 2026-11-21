// Configuration parsing and resolution tests. We trust that LLVM CLI library is
// well-tested, so we won't test CLI parsing too much.

#include "importizer/Config.hh"
#include "testing/Doctest.hh"
#include "utils/Diag.hh"
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/FormatVariadic.h>
#include <llvm/Support/VirtualFileSystem.h>
#include <llvm/Support/YAMLTraits.h>
#include <llvm/Support/raw_ostream.h>
#include <array>
#include <system_error>

namespace {

auto addYMLConfig(vfs::InMemoryFileSystem &memFS, CLIConfig &cliCfg, const YMLConfig &ymlCfg)
    -> llvm::Error {
  llvm::SmallString<64> ymlContent;
  llvm::raw_svector_ostream stream{ymlContent};
  // NOLINTBEGIN(cppcoreguidelines-pro-type-const-cast): Won't modify the CLI or YML config
  yml::Output ymlOut{stream, const_cast<void *>(static_cast<const void *>(&cliCfg)), 100};
  ymlOut << const_cast<YMLConfig &>(ymlCfg);
  // NOLINTEND(cppcoreguidelines-pro-type-const-cast)
  if (std::error_code const err{ymlOut.error()}) {
    return llvm::make_error<EmptyErr>();
  }
  return memFS.addFile(cliCfg.configPath, 0, llvm::MemoryBuffer::getMemBufferCopy(ymlContent, {}))
             ? llvm::Error::success()
             : llvm::createStringError(
                   llvm::formatv("File {} exists with different content.", cliCfg.configPath));
}

} // namespace

SUITE("Config");

CASE("CLI parsing") {
  vfs::InMemoryFileSystem memFS;

  SUBCASE("Default --write") {
    std::array<char const *, 1> argv{"importizer"};
    llvm::Expected<CLIConfig> cliCfg{getCLIConfig(argv.size(), argv.data(), memFS)};
    REQUIRE(cliCfg);
    CHECK_FALSE(cliCfg->write);
  }
}

CASE("Resolution") {
  vfs::InMemoryFileSystem memFS;
  CLIConfig cliCfg;
  cliCfg.configPath = "/Config.yml";
  YMLConfig ymlCfg;

  SUBCASE("Bad configurations") {
    SUBCASE("Nonexistent config file") {
      // Easiest way to satisfy YAML validation
      ymlCfg.compDBPath.emplace("");
    }

    SUBCASE("Invalid root") {
      ymlCfg.compDBPath.emplace("");
      ymlCfg.rootDir = "";
    }

    SUBCASE("Invalid globs") {
      ymlCfg.compDBPath.emplace("");
      ymlCfg.rootDir = "/";
      ymlCfg.hdrGlobExprs = {"[", "]"};
    }

    SUBCASE("Nonexistent JSON compilation database") {
      ymlCfg.compDBPath.emplace("");
      ymlCfg.rootDir = "/";
      ymlCfg.hdrGlobExprs = {"*.hh"};
    }

    SUBCASE("Invalid explicit source globs") {
      ymlCfg.rootDir = "/";
      ymlCfg.hdrGlobExprs = {"*.hh"};
      YMLExplicit &xplicit{ymlCfg.xplicit.emplace()};
      xplicit.srcGlobExprs = {"[", "]"};
    }

    llvm::cantFail(addYMLConfig(memFS, cliCfg, ymlCfg));
    CHECK_FALSE(resolveConfig(cliCfg, memFS));
  }
}

END_SUITE();
