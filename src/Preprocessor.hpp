#pragma once
#include "Directive.hpp"
#include <vector>

namespace re {
  class Pattern;
}
struct File;
struct Opts;

struct PreprocessResult {
  std::vector<Directive> directives;
};

PreprocessResult preprocess(File& file, const re::Pattern& maybeIncludeGuardPat);