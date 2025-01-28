#include "Export.hpp"
#ifdef CPP_MODULES
module;
#include <string>
#include <vector>
#include <variant>
export module CondMinimizer;
import Directive;
#else
#include "Directive.cppm"
#include <string>
#include <vector>
#include <variant>
#endif


using MinimizeCondCtx = std::vector<std::variant<std::string, Directive>>;

std::string minimizeCondToStr(MinimizeCondCtx& mcCtx);