#include "importizer/Main.hh"
#include "importizer/Opts.hh"
#include <clang/Tooling/JSONCompilationDatabase.h>
#include <cstdlib>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/raw_ostream.h>

int importizerMain(const int argc, const char *const *argv) {
  Opts opts;
  if (!getOpts(argc, argv, opts)) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
