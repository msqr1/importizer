#include "utils/Log.hh"
#include <string_view>
#ifdef _WIN32
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
  // 1. Base State: TTY check
#ifdef _WIN32
  HANDLE h{GetStdHandle(STD_ERROR_HANDLE)};
  DWORD mode;
  raw = !GetConsoleMode(h, &mode);
#else
  raw = !isatty(fileno(stderr));
#endif

  // 2. Environment variable override
#ifdef _WIN32
  DWORD res{GetEnvironmentVariableA("RAW", nullptr, 0)};
  if (res > 0) {
    raw = true;
  } else {
    DWORD errCode{GetLastError()};
    // 203 is ERROR_ENVVAR_NOT_FOUND. If it's missing, ignore.
    if (errCode != 0 && errCode != 203) {
      err("Unable to check environment variable 'RAW': {}",
          std::system_category().message(errCode));
      return false;
    }
  }
#else
  const char *envRaw{std::getenv("RAW")};
  raw |= envRaw != nullptr && envRaw[0] != '\0';
#endif

  // 3. CLI option override
  std::string_view arg;
  for (int i = 1; i < argc; ++i) {
    arg = argv[i];
    if (arg == "-r" || arg == "--raw") {
      if (++i >= argc) {
        raw = true;
        break; // Reached the end, default to true
      }
      std::string_view val{argv[i]};
      if (val == "always") {
        raw = true;
      } else if (val == "never") {
        raw = false;
      } else {
        err("Unknown value {} for '{}'", val, arg);
        return false;
      }
      break; // Found our flag, skip the rest
    }
  }
  return true;
}

#if defined(__has_feature) && __has_feature(address_sanitizer)
extern "C" const char *__asan_default_options() {
  return "log_path=stdout"
#if defined(__x86_64__) && defined(__APPLE__)
         // LSAN is only for x64 Linux & MacOS, but it's only on by default for
         // Linux so we explicitly turn it on for MacOS
         ":detect_leaks=1"
#endif
      ;
}
#endif
