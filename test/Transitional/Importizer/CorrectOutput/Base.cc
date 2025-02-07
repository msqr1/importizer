#ifdef CPP_MODULES
module;
#endif
#include "Export.hpp"
#ifdef CPP_MODULES
module Base;
#else
#include "Base.cppm"
#endif


bool verbose;
void exitOK() {
  throw 0;
}