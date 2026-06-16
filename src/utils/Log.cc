#include "utils/Log.hh"

LogOpts *logOpts;

#if defined(__has_feature) && __has_feature(address_sanitizer)
extern "C" const char *__asan_default_options() {
  return "log_path=stdout"
#if defined(__x86_64__) && defined(__APPLE__)
         // LSAN is only for amd64 Linux & MacOS, but it's only on by default
         // for Linux so we explicitly turn it on for MacOS
         ":detect_leaks=1"
#endif
      ;
}
#endif
