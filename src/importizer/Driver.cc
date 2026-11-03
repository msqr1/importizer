#include "importizer/Main.hh"
#include "utils/Diag.hh"
#include <llvm/Support/Error.h>
#include <llvm/Support/raw_ostream.h>

auto main(const int argc, const char *const *argv) -> int {
  // Always set program and log stream before everything
  setGlobalLogOpts({.prog = "importizer", .stream = &llvm::errs()});
  llvm::handleAllErrors(importizerMain(argc, argv),
                        [](const llvm::ErrorInfoBase &errInfo) -> llvm::Error {
                          err("{}", errInfo.message());
                          return llvm::Error::success();
                        });
}
