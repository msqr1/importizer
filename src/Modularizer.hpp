#pragma once
#include "Lexer.hpp"
#include <vector>

struct Opts;
struct File;
bool modularize(File& file, const LexResult& lexRes, const Opts& opts); 