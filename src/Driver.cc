#include "Opts.hh"
#include "Util.hh"
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
    err("{}\n", e.what());
    return EXIT_FAILURE;
  }
}
