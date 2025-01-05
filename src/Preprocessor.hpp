#pragma once
#include "Directive.hpp"
#include <vector>
#include <string>
#include <variant>

namespace re {
  class Pattern;
}
struct File;
struct Opts;

struct PreprocessResult {
  std::vector<Directive> directives;
};

PreprocessResult preprocess(File& file, const std::optional<re::Pattern>& maybeIncludeGuardPat);