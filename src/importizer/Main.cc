#include "importizer/Main.hh"
#include "importizer/Opts.hh"
#include <cstdlib>

int importizerMain(const int argc, const char *const *argv) {
  Opts opts;
  if (!getOpts(argc, argv, opts)) {
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
