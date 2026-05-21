#include "Util.hpp"
#include <cstdlib>

void exitOk() { throw EXIT_SUCCESS; }
void unreachable() {
#if defined(_MSC_VER) && !defined(__clang__) // MSVC
  __assume(false);
#else // GCC, Clang
  __builtin_unreachable();
#endif
}
