#include "importizer/Entry.hh"
#include "importizer/Opts.hh"
#include <cassert>
#include <cstdlib>
#include <llvm/Support/FileSystem.h>

int entry(const int argc, const char *const *argv) {
  Opts opts;
  if (!getOpts(argc, argv, opts)) {
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
