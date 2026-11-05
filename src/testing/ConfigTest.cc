// Configuration parsing and resolution tests. We trust that LLVM CLI library is well-tested, so we
// won't test CLI parsing too much.

#include "importizer/Config.hh"
#include "testing/Unit.hh"
#include "utils/Diag.hh"
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Twine.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/FormatVariadic.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/VirtualFileSystem.h>
#include <llvm/Support/YAMLTraits.h>
#include <llvm/Support/raw_ostream.h>
#include <system_error>

namespace {

auto addYMLConfig(vfs::InMemoryFileSystem &memFS, CLIConfig &cliCfg, const YMLConfig &ymlCfg)
    -> llvm::Error {
  llvm::SmallString<64> ymlContent;
  llvm::raw_svector_ostream stream{ymlContent};
  // NOLINTBEGIN
  yml::Output ymlOut{stream, const_cast<void *>(static_cast<const void *>(&cliCfg)), 100};
  ymlOut << const_cast<YMLConfig &>(ymlCfg);
  // NOLINTEND
  if (std::error_code err{ymlOut.error()}) {
    return llvm::make_error<EmptyErr>();
  }
  return memFS.addFile(cliCfg.configPath, 0, llvm::MemoryBuffer::getMemBufferCopy(ymlContent, {}))
             ? llvm::Error::success()
             : llvm::createStringError(
                   llvm::formatv("File {} exists with different content.", cliCfg.configPath));
}

} // namespace

TEST_SUITE_BEGIN("Config");

TEST_CASE("CLI parsing") {}

TEST_CASE("Resolution") {
  llvm::Error err{llvm::Error::success()};
  CLIConfig cliCfg;
  cliCfg.configPath = "/Config.yml";

  SUBCASE("Nonexistant config file") {
    vfs::InMemoryFileSystem memFS;
    CHECK_UNARY_FALSE(!!resolveConfig(cliCfg, memFS));
  }

  SUBCASE("Invalid root") {
    vfs::InMemoryFileSystem memFS;
    bool setupOk{true};
    setupOk &= addYMLConfig(
        memFS, cliCfg, {.xplicit{}, .compDBPath{""}, .hdrGlobExprs{}, .rootDir{""}, .stdImport{}});
    if (!setupOk) {
      FAIL("Setup failed.");
      return;
    }

    CHECK_UNARY_FALSE(!!resolveConfig(cliCfg, memFS));
  }
}

TEST_SUITE_END();
