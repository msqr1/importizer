#pragma once
#include "OptProcessor.hpp"
#include "FileOp.hpp"
#include "Directive.hpp"
#include <vector>

bool insertPreamble(File& file, std::vector<Directive>&& directives, const Opts& opts);