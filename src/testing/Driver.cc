#include "testing/E2E.hh"
#include "testing/Unit.hh"
#include "utils/Diag.hh"
#include <cassert>
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Twine.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/raw_ostream.h>
#include <string>

namespace cl = llvm::cl;

auto main(const int argc, const char *const *argv) -> int {
  // Always set program and log stream before everything
  setGlobalLogOpts({.prog = "test-tool", .stream = &llvm::errs()});

  // LLVM default options will mix into ours if we don't make our own category
  cl::OptionCategory cat{globalLogOpts.prog};

  cl::SubCommand runE2E{"run-e2e", "Run an end-to-end test."};
  cl::SubCommand updateE2E{"update-e2e", "Update an end-to-end test."};
  cl::SubCommand runUnit{"run-unit", "Run a unit test suite."};
  cl::opt<std::string> suiteName{
      cl::cat{cat}, cl::sub{runUnit}, cl::desc{"<suite name>"}, cl::Positional, cl::ValueRequired,
  };
  cl::list<std::string> doctestArgs{
      cl::cat{cat},
      cl::sub{runUnit},
      cl::desc{"<doctest args>..."},
      cl::ConsumeAfter,
  };
  cl::opt<std::string> testDir{
      cl::cat{cat},   cl::sub{runE2E},   cl::sub{updateE2E}, cl::desc{"<test directory>"},
      cl::Positional, cl::ValueRequired,
  };
  cl::opt<std::string> outDir{
      cl::cat{cat}, cl::sub{runE2E}, cl::desc{"<out directory>"}, cl::Positional, cl::ValueRequired,
  };
  cl::HideUnrelatedOptions(cat);
  llvm::DenseMap<llvm::StringRef, cl::Option *> &optMap{cl::getRegisteredOptions()};
  optMap["help"]->setDescription("Display available options.");
  optMap["help-list"]->setDescription("List available options.");
  optMap["version"]->setHiddenFlag(cl::ReallyHidden);
  if (!cl::ParseCommandLineOptions(argc, argv, "Test tool of importizer", globalLogOpts.stream)) {
    return 1;
  }

  if (runE2E) {
    llvm::handleAllErrors(runE2ETest(testDir, outDir), ErrorLogger{});
  } else if (updateE2E) {
    llvm::handleAllErrors(updateE2ETest(testDir), ErrorLogger{});
  } else if (runUnit) {
    return runUnitTest(suiteName.c_str(), doctestArgs);
  }
}
