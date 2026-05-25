#include "utils/Log.hh"
#include <cstdlib>
#include <stdio.h>
#include <string_view>
#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

bool color;
bool getColor(int argc, const char **argv) noexcept {
  // TTY check
#ifdef WIN32
  color = _isatty(_fileno(stderr));
#else
  color = isatty(fileno(stderr));
#endif

  // NO_COLOR env var
  std::string_view noColorEnv{std::getenv("NO_COLOR")};
  color = !noColorEnv.empty();
  // User option
  for (int i = 1; i < argc; ++i) {
    std::string_view arg = argv[i];
    if (arg == "-c" || arg == "--color") {
      if (++i >= argc) {
        err("{} requires a value\n", arg);
        return false;
      }
      std::string_view val = argv[i];
      if (val == "always") {

      } else if (val == "never") {

      } else {
        err("Invalid value '{}' for {}", val, arg);
        return false;
      }
    }
  }
  return true;
}
