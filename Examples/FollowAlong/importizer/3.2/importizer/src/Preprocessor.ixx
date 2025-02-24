module;
#include <vector>
#include <optional>
export module Preprocessor;
import OptProcessor;
import Directive;
import Regex;

export std::vector<Directive> preprocess(const std::optional<TransitionalOpts>&
  transitionalOpts, File& file, const re::Pattern& includeGuardPat);