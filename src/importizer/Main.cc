#include "importizer/Main.hh"
#include "importizer/ClangDiag.hh"
#include "importizer/Frontend.hh"
#include "importizer/Opts.hh"
#include "utils/Diag.hh"
#include <cassert>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/Error.h>
#include <memory>

namespace tl = clang::tooling;

auto importizerMain(const int argc, const char *const *argv) -> llvm::Error {
  llvm::Expected<Opts> opts{getOpts(argc, argv)};
  if (!opts) {
    return opts.takeError();
  }
  tl::ClangTool tool{*opts->compDB, opts->srcs};
  FrontendCtx ctx{*opts};
  FrontendAnalyzerFactory factory{ctx};
  std::unique_ptr<MyDiagnosticConsumer> diagHandler{std::make_unique<MyDiagnosticConsumer>()};
  tool.setDiagnosticConsumer(diagHandler.get());
  switch (tool.run(&factory)) {
  case 0:
    break;
  case 1:
    return llvm::make_error<EmptyErr>();
  case 2:
    assert(false && "Skipped files, missing compile commands??");
    break;
  default: // Other file types
    break;
  }
  return llvm::Error::success();
}
