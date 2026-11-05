// Configuration parsing and resolution tests. We trust that LLVM CLI library is well-tested, so we
// won't test CLI parsing too much.

#include "importizer/Config.hh"
#include "testing/Unit.hh"
#include <llvm/Support/Error.h>

TEST_SUITE_BEGIN("Config");

TEST_CASE("Resolution") {
  CLIConfig cliCfg;
  cliCfg.configPath = "/Config.yml";

  SUBCASE("Nonexistant config file") {
    llvm::Expected<Config> cfg{resolveConfig(cliCfg)};
    CHECK_UNARY_FALSE(!!cfg);
  }
}

TEST_SUITE_END();
