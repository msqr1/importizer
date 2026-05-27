#include "utils/Log.hh"
#include <string_view>
#ifdef WIN32
#include <system_error>
#include <win32/io.h>
#include <win32/misc.h>
#include <win32/windows_base.h>
#else
#include <cstdlib>
#include <stdio.h>
#include <unistd.h>
#endif

bool raw;
bool getRaw(int argc, const char **argv) noexcept {
  // Auto: TTY check
#ifdef WIN32
  HANDLE h{GetStdHandle(STD_ERROR_HANDLE)};
  DWORD mode;
  raw = !GetConsoleMode(h, &mode);
#else
  raw = !isatty(fileno(stderr));
#endif

  // RAW env var
#ifdef WIN32
  GetEnvironmentVariableA("RAW", nullptr, 1);
  DWORD errCode{GetLastError()};
  if (errCode != 0) {
    // ERROR_ENVVAR_NOT_FOUND
    if (errCode == 203) {
      raw = false;
    } else [[unlikely]] {
      err("Unable to check environment variable 'RAW': {}\n",
          std::system_category().message(errCode));
    }
  } else {
    raw = true;
  }
#else
  // const char* here instead bc it can be nullptr
  const char *envRaw{std::getenv("RAW")};
  if (envRaw != nullptr && envRaw[0] != '\0') {
    raw = true;
  }
#endif

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
