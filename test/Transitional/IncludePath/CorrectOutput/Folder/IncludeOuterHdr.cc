#ifdef CPP_MODULES
module;
#endif
#include "../Export.hpp"
#ifdef CPP_MODULES
export module Folder.IncludeOuterHdr;
import Header;
#else
#include <Header.cppm>
#endif

