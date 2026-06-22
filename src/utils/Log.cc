#include "utils/Log.hh"

LogOpts *logOpts;

#ifdef __has_feature
#if __has_feature(address_sanitizer)
extern "C" const char *__asan_default_options() {
  return ""
#if defined(__x86_64__) && defined(__APPLE__)
         // LSAN is only for amd64 Linux & MacOS, but it's only on by default
         // for Linux so we explicitly turn it on for MacOS
         ":detect_leaks=1"
#endif
      ;
}
#endif
#endif
