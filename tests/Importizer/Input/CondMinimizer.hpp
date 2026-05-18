#include "Directive.hpp"
#include <string>
#include <vector>
#include <variant>

// Just to know that only IfCond, Else, ElCond, and EndIf are allowed
using CondDirective = Directive;
using MinimizeCondCtx = std::vector<std::variant<std::string, CondDirective>>;

std::string minimizeCondToStr(MinimizeCondCtx& mcCtx);