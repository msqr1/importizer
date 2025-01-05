#pragma once
#include <vector>

struct Opts;
struct File;
struct PreprocessResult;

bool modularize(File& file, const PreprocessResult& prcRes, const Opts& opts);