module;
#include <vector>
export module Preamble;
import OptProcessor;
import FileOp;
import Directive;


bool insertPreamble(File& file, std::vector<Directive>&& directives, const Opts& opts);