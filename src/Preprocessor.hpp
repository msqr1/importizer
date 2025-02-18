#pragma once
#include "OptProcessor.hpp"
#include "Directive.hpp"
#include <cstdint>
#include <vector>

struct File;
struct Opts;
struct PreprocessRes {

  // In SOF comments, 1 for a preamble after a singleline, 2 for a preamble after a multiline
  uint8_t prefixNewlineCnt{}; 
  uintmax_t insertionPos{};
  std::vector<Directive> directives;
};
PreprocessRes preprocess(const Opts& opts, File& file);