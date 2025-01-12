#pragma once
#include "ArgProcessor.hpp"
#include "Directive.hpp"
#include <vector>
#include <optional>

namespace re {
  class Pattern;
}
struct File;
struct Opts;
struct TransitionalOpts;

struct PreprocessResult {
  std::vector<Directive> directives;
};

PreprocessResult preprocess(const std::optional<TransitionalOpts>& transitionalOpts, File& file, const re::Pattern& includeGuardPat);