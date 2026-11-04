#include "importizer/Main.hh"
#include "importizer/ClangDiag.hh"
#include "importizer/Config.hh"
#include "importizer/Frontend.hh"
#include <cassert>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/Error.h>
#include <memory>

namespace tl = clang::tooling;

auto importizerMain(const int argc, const char *const *argv) -> llvm::Error {
  llvm::Expected<Config> cfg{resolveConfig(argc, argv)};
  if (!cfg) {
    return llvm::joinErrors(cfg.takeError(),
                            llvm::createStringError("When resolving configuration"));
  }
  tl::ClangTool tool{*cfg->compDB, cfg->srcs};
  FrontendCtx ctx{*cfg};
  FrontendAnalyzerFactory factory{ctx};
  std::unique_ptr<MyDiagnosticConsumer> diagHandler{std::make_unique<MyDiagnosticConsumer>()};
  tool.setDiagnosticConsumer(diagHandler.get());
  switch (tool.run(&factory)) {
  case 0:
    break;
  case 1:
    return llvm::createStringError("When running tool");
  case 2:
    assert(false && "Skipped files because of missing compile commands??");
    break;
  default: // Other file types
    break;
  }
  return llvm::Error::success();
}
