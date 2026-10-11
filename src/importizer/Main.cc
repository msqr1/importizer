#include "importizer/Main.hh"
#include "importizer/Diagnostics.hh"
#include "importizer/Frontend.hh"
#include "importizer/Opts.hh"
#include <cassert>
#include <clang/Tooling/Tooling.h>
#include <cstdlib>
#include <memory>

namespace tl = clang::tooling;

int importizerMain(const int argc, const char *const *argv) {
  Opts opts;
  if (!getOpts(argc, argv, opts)) {
    return EXIT_FAILURE;
  }
  tl::ClangTool tool{*opts.compDB, opts.srcs};
  FrontendInfo info;
  FrontendAnalyzerFactory factory{info};
  std::unique_ptr<MyDiagnosticConsumer> diagHandler{
      std::make_unique<MyDiagnosticConsumer>()};
  tool.setDiagnosticConsumer(diagHandler.get());
  switch (tool.run(&factory)) {
  case 0: // No error
    break;
  case 1: // Yes error
    return EXIT_FAILURE;
  case 2: // Skipped files (impossible)
    assert(false);
  };
  return EXIT_SUCCESS;
}
