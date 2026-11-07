#include "importizer/Main.hh"
#include "importizer/ClangDiag.hh"
#include "importizer/Config.hh"
#include "importizer/Frontend.hh"
#include "utils/Diag.hh"
#include <cassert>
#include <clang/Tooling/Tooling.h>
#include <cstdlib>
#include <llvm/Support/Error.h>
#include <memory>
#include <utility>

namespace tl = clang::tooling;

namespace {

auto importizerMain2(const int argc, const char *const *argv) -> llvm::Error {
  llvm::Expected<CLIConfig> cliCfg{getCLIConfig(argc, argv)};
  if (!cliCfg) {
    return llvm::joinErrors(cliCfg.takeError(),
                            llvm::createStringError("When parsing CLI options."));
  }

  llvm::Expected<Config> cfg{resolveConfig(*cliCfg)};
  if (!cfg) {
    return llvm::joinErrors(cfg.takeError(),
                            llvm::createStringError("When resolving configuration."));
  }
  tl::ClangTool tool{*cfg->compDB, cfg->srcs};
  FrontendCtx ctx{*cfg};
  FrontendAnalyzerFactory factory{ctx};
  tool.setDiagnosticConsumer(std::make_unique<MyDiagnosticConsumer>().get());
  switch (tool.run(&factory)) {
  case 0:
    break;
  case 1:
    return llvm::createStringError("When running tool.");
  case 2:
    assert(false && "Skipped files because of missing compile commands??");
    break;
  default: // Other file types
    break;
  }
  return llvm::Error::success();
}

} // namespace

auto importizerMain(const int argc, const char *const *argv) -> int {
  if (llvm::Error err{importizerMain2(argc, argv)}) {
    llvm::handleAllErrors(std::move(err), ErrorLogger{});
    return EXIT_FAILURE;
  }
  return EXIT_FAILURE;
}
