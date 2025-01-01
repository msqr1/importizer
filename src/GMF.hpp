#pragma once
#include "Directive.hpp"
#include <vector>

struct Opts;
struct File;
void insertGMF(File& file, const std::vector<Directive>& directives, const Opts& opts);