#pragma once
#include "Export.hpp"
#ifdef CPP_MODULES
module;
#include <vector>
#include <optional>
export module Preprocessor;
import OptProcessor;
import Directive;
#else
#include "OptProcessor.cppm"
#include "Directive.cppm"
#include <vector>
#include <optional>
#endif


namespace re {
  class Pattern;
}
struct File;
struct Opts;
struct TransitionalOpts;
std::vector<Directive> preprocess(const std::optional<TransitionalOpts>& transitionalOpts,
  File& file, const re::Pattern& includeGuardPat);