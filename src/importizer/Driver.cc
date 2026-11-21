#include "importizer/Main.hh"
#include "utils/Diag.hh"
#include <llvm/Support/raw_ostream.h>

auto main(int const argc, char const *const *argv) -> int {
  // Always set program and log stream before everything
  globalLogCfg.set("importizer", &llvm::errs());

  return importizerMain(argc, argv);
}
