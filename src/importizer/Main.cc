#include "importizer/Main.hh"
#include "importizer/Diagnostics.hh"
#include "importizer/Frontend.hh"
#include "importizer/Opts.hh"
#include "clang/Tooling/Tooling.h"
#include <cassert>
#include <cstdlib>
#include <memory>

namespace tl = clang::tooling;

int importizerMain(const int argc, const char *const *argv) {
  Opts opts;
  if (!getOpts(argc, argv, opts)) {
    return EXIT_FAILURE;
  }
  tl::ClangTool tool{*opts.compDB, opts.srcs};
  FrontendInfo info{opts};
  FrontendAnalyzerFactory factory{info};
  std::unique_ptr<MyDiagnosticConsumer> diagHandler{
      std::make_unique<MyDiagnosticConsumer>()};
  tool.setDiagnosticConsumer(diagHandler.get());
  switch (tool.run(&factory)) {
  case 0: // No error
    break;
  case 1: // Yes error
    return EXIT_FAILURE;
  case 2:
    assert(false && "Skipped files, missing compile commands??");
  };
  return EXIT_SUCCESS;
}
