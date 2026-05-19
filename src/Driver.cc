#include "Opts.hpp"
#include <cstdlib>
int main(const int argc, const char **argv) {
  try {
    Opts opts{getOpts(argc, argv)};
  } catch (int code) {
    return code;
  }
  return EXIT_SUCCESS;
}
