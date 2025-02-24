module;
#include <vector>
export module Preamble;
import OptProcessor;
import FileOp;
import Directive;

export bool insertPreamble(File& file, std::vector<Directive>&& directives,
  const Opts& opts);