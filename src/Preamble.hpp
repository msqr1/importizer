#pragma once
#include "Directive.hpp"
#include <vector>

struct Opts;
struct File;
bool addPreamble(File& file, std::vector<Directive>&& directives, const Opts& opts);