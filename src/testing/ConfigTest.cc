// Configuration parsing and resolution tests. We trust that LLVM CLI library is well-tested, so we
// won't test CLI parsing too much.

#include "importizer/Config.hh"
#include "testing/Unit.hh"
#include <llvm/Support/Error.h>

TEST_SUITE_BEGIN("Config");

TEST_CASE("Resolution") {
  CLIConfig cliCfg;
  cliCfg.configPath = "/Config.yml";

  SUBCASE("Nonexistant config file") { CHECK_UNARY_FALSE(!!resolveConfig(cliCfg)); }
}

TEST_SUITE_END();
