#include "Export.hpp"
#ifdef CPP_MODULES
module;
#if __has_include(<iostream>)
#include <iostream>
#endif
#ifdef COND1
#include "RandomHeader.hpp"
#endif
#ifdef COND2
#define a
#endif
export module Test;
#ifdef COND2
#define a
#endif
#else
#if __has_include(<iostream>)
#include <iostream>
#endif
#ifdef COND1
#include "RandomHeader.hpp"
#endif
#ifdef COND2
#define a
#endif
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
#else
// Do something else
#endif