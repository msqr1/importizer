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
  // Always set program and log stream before everything, in this case, we suppress everything.
  LogOpts logOpts{.prog = "", .stream = &llvm::nulls()};
  setGlobalLogOpts(logOpts);

  doctest::Context ctx;
  std::stringstream doctestOut;
  ctx.setOption("ts", suiteName);
  ctx.setCout(&doctestOut);
  const int doctestRtn{ctx.run()};
  llvm::outs() << doctestOut.rdbuf();
  return doctestRtn;
}
