#pragma once
#include "OptProcessor.hpp"
#include "Directive.hpp"
#include <vector>

struct File;
struct Opts;
std::vector<Directive> preprocess(const Opts& opts, File& file);