#pragma once
#ifdef CPP_MODULES
module;
#endif
#include "Export.hpp"
#include <vector>
#include <optional>
#ifdef CPP_MODULES
export module Preprocessor;
import OptProcessor;
import Directive;
#else
#include "OptProcessor.ixx"
#include "Directive.ixx"
#endif


namespace re {
  class Pattern;
}
struct File;
struct Opts;
struct TransitionalOpts;
std::vector<Directive> preprocess(const std::optional<TransitionalOpts>& transitionalOpts,
  File& file, const re::Pattern& includeGuardPat);