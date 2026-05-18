module;
#if __has_include(<iostream>)
#include <iostream>
#endif
#ifdef COND3
#define A
#endif
#ifdef COND4
#define B
#endif
export module Test;
import LocalHdr;

// Should not be minimized away
#if __has_include(<iostream>)
#endif

// Should be eliminated away
#ifdef COND1
#endif

// Should be removed completely
#ifdef COND2
#ifdef NESTED
#endif
#endif

// The elifdef should be removed
#ifdef COND3
#define A
#elifdef COND3
#ifdef NESTED
#endif
#endif

// The else should be removed
#ifdef COND4
#define B
#else
#endif

// Should be removed
#define C
#undef C