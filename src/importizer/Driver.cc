#include "importizer/Opts.hh"
#include "utils/Control.hh"
#include <cassert>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <string_view>
#include <system_error>

std::string_view prog() { return "importizer"; }

void run(const int argc, const char **argv) {
  Opts opts;
  getOpts(argc, argv, opts);
  std::error_code errCode;
  fs::recursive_directory_iterator it{opts.inDir, errCode};
  if (errCode) {
    exitWithErr("Unable to iterate {}: {}\n", opts.inDir, errCode.message());
  }
  for (const fs::directory_entry &entry : it) {
    if (!entry.is_regular_file()) {
      continue;
    }
  }
}

int main(const int argc, const char **argv) {
  try {
    run(argc, argv);
  } catch (const int code) {
    return code;
  } catch (const std::exception &e) {
    err("{}\n", e.what());
    return EXIT_FAILURE;
  }
}
