#pragma once
#include "Directive.hpp"
#include <vector>

struct Opts;
struct File;
bool insertPreamble(File& file, std::vector<Directive>&& directives, const Opts& opts);