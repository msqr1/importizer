#pragma once
#include "Export.hpp"
#ifdef CPP_MODULES
module;
#include <vector>
export module Preamble;
import Directive;
#else
#include "Directive.cppm"
#include <vector>
#endif


struct Opts;
struct File;
bool insertPreamble(File& file, std::vector<Directive>&& directives, const Opts& opts);