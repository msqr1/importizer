#pragma once
#include "OptProcessor.hpp"
#include "Directive.hpp"
#include "FileOp.hpp"
#include "Regex.hpp"
#include <vector>
#include <optional>

std::vector<Directive> preprocess(const std::optional<TransitionalOpts>& transitionalOpts,
  File& file, const re::Pattern& includeGuardPat);