module;
#include <vector>
#include <optional>
export module Preprocessor;
import OptProcessor;
import Directive;


namespace re {
  class Pattern;
}
struct File;
struct Opts;
struct TransitionalOpts;
std::vector<Directive> preprocess(const std::optional<TransitionalOpts>& transitionalOpts,
  File& file, const re::Pattern& includeGuardPat);