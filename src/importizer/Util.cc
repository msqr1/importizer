#include "importizer/Util.hh"
#include "fmt/base.h"
#include <cstdlib>

void exitOk() { throw EXIT_SUCCESS; }
void progPrefix() { fmt::print(stderr, "importizer: "); }
