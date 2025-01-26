#include "../../Export.hpp"
#ifdef CPP_MODULES
module;
export module Folder.IncludeOuterHdr;
import Header;
#else
#include <Header.hpp>
#endif

