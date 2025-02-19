#ifdef CPP_MODULES
module;
#endif
#include "Export.hpp"
#include <string>
#include <vector>
#include <variant>
#ifdef CPP_MODULES
export module CondMinimizer;
import Directive;
#else
#include "Directive.ixx"
#endif


// Just to know that only IfCond, Else, ElCond, and EndIf are allowed
using CondDirective = Directive;
using MinimizeCondCtx = std::vector<std::variant<std::string, CondDirective>>;

std::string minimizeCondToStr(MinimizeCondCtx& mcCtx);