#ifdef CPP_MODULES
module;
#endif
#include "Export.hpp"
#if __has_include(<iostream>)
#include <iostream>
#endif
#ifdef COND1
#include "RandomHeader.hpp"
#endif
#ifdef CPP_MODULES
export module Test;
#else
#endif

#ifdef COND 
#endif 
#if __has_include(<iostream>)
#endif
#ifdef COND1
#endif
#ifdef COND2
#define a
#elifdef b 
#else 
#endif
#ifdef COND3
// Do something
#ifdef NESTED
// Do something else
#endif
#endif
#define A
#undef A