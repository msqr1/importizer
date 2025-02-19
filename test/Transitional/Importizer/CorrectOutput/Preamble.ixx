#pragma once
#ifdef CPP_MODULES
module;
#endif
#include "Export.hpp"
#include <vector>
#ifdef CPP_MODULES
export module Preamble;
import Directive;
#else
#include "Directive.ixx"
#endif


struct Opts;
struct File;
bool insertPreamble(File& file, std::vector<Directive>&& directives, const Opts& opts);