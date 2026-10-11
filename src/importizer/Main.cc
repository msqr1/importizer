#include "importizer/Main.hh"
#include "importizer/Frontend.hh"
#include "importizer/Opts.hh"
#include <cassert>
#include <clang/Tooling/Tooling.h>
#include <cstdlib>

int importizerMain(const int argc, const char *const *argv) {
  Opts opts;
  if (!getOpts(argc, argv, opts)) {
    return EXIT_FAILURE;
  }
  tl::ClangTool tool{*opts.compDB, opts.srcs};
  switch (tool.run(tl::newFrontendActionFactory<FrontendAnalyzer>().get())) {
  case 0: // No error
    break;
  case 1: // Yes error
    return EXIT_FAILURE;
  case 2: // Skipped files (impossible)
    assert(false);
  };
  return EXIT_SUCCESS;
}
