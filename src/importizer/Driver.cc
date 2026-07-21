#include "importizer/Main.hh"
#include "utils/Log.hh"
#include <llvm/Support/raw_ostream.h>

int main(const int argc, const char *const *argv) {
  // Always set program & log target before everything
  LogOpts logOpts;
  g_logOpts = &logOpts;
  logOpts.prog = "importizer";
  logOpts.target = &llvm::errs();
  return importizerMain(argc, argv);
}
