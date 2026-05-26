#include "utils/Log.hh"
#include "fmt/base.h"
#include <cstdlib>
#include <stdio.h>
#include <string_view>
#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

bool raw;
bool getRaw(int argc, const char **argv) noexcept {

  // Auto: TTY check
#ifdef WIN32
  raw = !_isatty(_fileno(stderr));
#else
  raw = !isatty(fileno(stderr));
#endif

  // RAW env var, const char* here instead bc it can be nullptr
  const char *envRaw = std::getenv("RAW");
  if (envRaw != nullptr && envRaw[0] != '\0') {
    raw = true;
  }

  // CLI option
  std::string_view arg;
  for (int i = 1; i < argc; ++i) {
    arg = argv[i];
    if (arg == "-r" || arg == "--raw") {
      if (++i >= argc) {
        raw = true;
        break;
      }
      std::string_view val{argv[i]};
      if (val == "always") {
        raw = true;
      } else if (val == "never") {
        raw = false;
      } else {
        err("Unknown value {} for '{}'\n", val, arg);
        return false;
      }
      break;
    }
  }
  return true;
}
