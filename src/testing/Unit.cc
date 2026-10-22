#include "testing/Unit.hh"
#include "utils/Diag.hh"
#include <sstream>
#define DOCTEST_CONFIG_IMPLEMENT
#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#define DOCTEST_CONFIG_NO_EXCEPTIONS
#define DOCTEST_CONFIG_NO_WINDOWS_SEH
#define DOCTEST_CONFIG_NO_POSIX_SIGNALS
#include <doctest/doctest.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/raw_ostream.h>

auto runUnitTest(const char *suiteName) -> int {
  // Always set program and log stream before everything
  LogOpts logOpts{.prog = "run-test unit", .stream = &llvm::errs()};
  setGlobalLogOpts(logOpts);

  doctest::Context ctx;
  std::stringstream doctestCLI;
  ctx.setOption("ts", suiteName);
  ctx.setCout(&doctestCLI);
  const int doctestRtn{ctx.run()};
  *logOpts.stream << doctestCLI.rdbuf();
  return doctestRtn;
}
