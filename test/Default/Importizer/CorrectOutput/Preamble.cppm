module;
#include <vector>
export module Preamble;
import Directive;


struct Opts;
struct File;
bool insertPreamble(File& file, std::vector<Directive>&& directives, const Opts& opts);