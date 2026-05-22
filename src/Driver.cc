#include "Opts.hh"
#include "fmt/base.h"
#include <cassert>
#include <cstdlib>
#include <exception>
#include <filesystem>

int main(const int argc, const char **argv) {
  try {
    Opts opts;
    getOpts(argc, argv, opts);
    for (const fs::directory_entry &entry :
         fs::recursive_directory_iterator{opts.inDir}) {
      if (!entry.is_regular_file()) {
        continue;
      }
    }
  } catch (int code) {
    return code;
  } catch (const std::exception &e) {
    fmt::println("{}", e.what());
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
