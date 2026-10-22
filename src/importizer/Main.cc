#include "importizer/Main.hh"
#include "importizer/ClangDiag.hh"
#include "importizer/Frontend.hh"
#include "importizer/Opts.hh"
#include <cassert>
#include <clang/Tooling/Tooling.h>
#include <cstdlib>
#include <llvm/Support/ErrorHandling.h>
#include <memory>

namespace tl = clang::tooling;

auto importizerMain(const int argc, const char *const *argv) -> int {
  Opts opts;
  if (!getOpts(argc, argv, opts)) {
    return EXIT_FAILURE;
  }
  tl::ClangTool tool{*opts.compDB, opts.srcs};
  FrontendCtx ctx{opts};
  FrontendAnalyzerFactory factory{ctx};
  std::unique_ptr<MyDiagnosticConsumer> diagHandler{std::make_unique<MyDiagnosticConsumer>()};
  tool.setDiagnosticConsumer(diagHandler.get());
  switch (tool.run(&factory)) {
  case 0: // No error
    break;
  case 1: // Yes error
    return EXIT_FAILURE;
  case 2:
    assert(false && "Skipped files, missing compile commands??");
    break;
  default: // Other file types
    break;
  }
  return EXIT_SUCCESS;
}
