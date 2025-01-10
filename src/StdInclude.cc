#include "StdInclude.hpp"
#include <array>
#include <optional>
#include <string_view>

namespace {

constexpr std::array<std::string_view, 114> cppOrCwrapHdrs {
  "cstdlib", "execution", "cfloat", "climits", "compare", "coroutine", "csetjmp",
  "csignal", "cstdarg", "cstddef", "cstdint", "exception", "initializer_list", "limits",
  "new", "source_location", "stdfloat", "typeindex", "typeinfo", "version", "concepts",
  "cassert", "cerrno", "debugging", "stacktrace", "stdexcept", "system_error", "memory",
  "memory_resource", "scoped_allocator", "ratio", "type_traits", "any", "bit", "bitset",
  "expected", "functional", "optional", "tuple", "utility", "variant", "array", "deque",
  "flat_map", "flat_set", "forward_list", "inplace_vector", "list", "map", "mdspan",
  "queue", "set", "span", "stack", "unordered_map", "unordered_set", "vector", "iterator",
  "generator", "ranges", "algorithm", "numeric", "cstring", "string", "string_view",
  "cctype", "charconv", "clocale", "codecvt", "cuchar", "cwchar", "cwctype", "format",
  "locale", "regex", "text_encoding", "cfenv", "cmath", "complex", "linalg", "numbers",
  "random", "simd", "valarray", "chrono", "ctime", "cinttypes", "cstdio", "filesystem",
  "fstream", "iomanip", "ios", "iosfwd", "iostream", "istream", "ostream", "print",
  "spanstream", "sstream", "streambuf", "strstream", "syncstream", "atomic", "barrier",
  "condition_variable", "future", "hazard_pointer", "latch", "mutex", "rcu", "semaphore",
  "shared_mutex", "stop_token", "thread"
};

constexpr std::array<std::string_view, 24> cCompatHdrs {
  "assert.h", "ctype.h", "errno.h", "fenv.h", "float.h", "inttypes.h", "limits.h", 
  "locale.h", "math.h", "setjmp.h", "signal.h", "stdarg.h", "stddef.h", "stdint.h", 
  "stdio.h", "stdlib.h", "string.h", "time.h", "uchar.h", "wchar.h", "wctype.h",
  "stdatomic.h", "complex.h", "tgmath.h"
};

}

std::optional<StdIncludeType> getStdIncludeType(std::string_view include) {
  for(std::string_view cppOrCwrapHdr : cppOrCwrapHdrs) {
    if(include == cppOrCwrapHdr) return StdIncludeType::CppOrCwrap;
  }
  for(std::string_view cCompatHdr : cCompatHdrs) {
    if(include == cCompatHdr) return StdIncludeType::CCompat;
  }
  return std::nullopt;
}