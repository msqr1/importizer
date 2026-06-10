#include "importizer/Opts.hh"
#include "utils/Log.hh"
#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <optional>
#include <string_view>
#include <system_error>

const std::string_view prog{"importizer"};

int main(const int argc, const char **argv) {
  if (!getRaw(argc, argv)) {
    return EXIT_FAILURE;
  };

  Opts opts;
  std::optional<bool> status{getOpts(argc, argv, opts)};
  if (!status) {
    return EXIT_SUCCESS;
  } else if (!*status) {
    return EXIT_FAILURE;
  }

  std::error_code errCode;
  fs::recursive_directory_iterator it{opts.inDir, errCode};
  if (errCode) {
    err("Unable to iterate {}: {}.", opts.inDir, errCode.message());
    return EXIT_FAILURE;
  }
  for (const fs::directory_entry &entry : it) {
    if (!entry.is_regular_file()) {
      continue;
    }
  }
}
