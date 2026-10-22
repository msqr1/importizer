#include "importizer/Main.hh"
#include "utils/Diag.hh"
#include <llvm/Support/raw_ostream.h>

auto main(const int argc, const char *const *argv) -> int {
  // Always set program and log stream before everything
  setGlobalLogOpts({.prog = "importizer", .stream = &llvm::errs()});

  return importizerMain(argc, argv);
}
