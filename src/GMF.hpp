#pragma once
#include "Directive.hpp"
#include <vector>
#include <string>

struct Opts;
void insertGMF(std::string& code, const std::vector<Directive>& directives, const Opts& opts);