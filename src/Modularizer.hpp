#pragma once
#include "Directive.hpp"
#include <vector>

struct Opts;
struct File;
bool modularize(File& file, const std::vector<Directive>& directives, const Opts& opts);