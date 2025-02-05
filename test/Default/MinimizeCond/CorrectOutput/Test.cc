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