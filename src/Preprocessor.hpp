#pragma once
#include "OptProcessor.hpp"
#include "Directive.hpp"
#include <vector>
#include <optional>

namespace re {
  class Pattern;
}
struct File;
struct Opts;
struct TransitionalOpts;

std::vector<Directive> preprocess(const std::optional<TransitionalOpts>& transitionalOpts, File& file, const re::Pattern& includeGuardPat);