#ifdef CPP_MODULES
module;
#endif
#include "Export.hpp"
#ifdef CPP_MODULES
export module UmbrellaHdr;
export import NormalHdr;
#else
#include "NormalHdr.cppm"
#endif

