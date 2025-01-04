#pragma once
#include "Preprocessor.hpp"
#include <vector>

struct Opts;
struct File;
bool modularize(File& file, const PreprocessResult& prcRes, const Opts& opts);