#include "Util.hpp"

void exitOK() {
  throw 0;
}
void unreachable() {
#if defined(_MSC_VER) && !defined(__clang__) // MSVC
  __assume(false);
#else // GCC, Clang
  __builtin_unreachable();
#endif
}