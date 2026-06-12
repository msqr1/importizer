#include "importizer/Entry.hh"
#include "utils/Log.hh"
#include <cstdlib>

int main(const int argc, const char *const *argv) {
  // Always set program and log target before doing things that can log
  LogOpts selfLogOpts;
  selfLogOpts.prog = "importizer";
  selfLogOpts.target = stderr;
  logOpts = &selfLogOpts;
  if (!getRaw(selfLogOpts.raw)) {
    return EXIT_FAILURE;
  };
  return entry(argc, argv);
}
