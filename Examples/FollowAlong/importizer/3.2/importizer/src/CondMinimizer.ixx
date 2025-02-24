module;
#include <string>
#include <vector>
#include <variant>
export module CondMinimizer;
import Directive;

export {

// Just to know that only IfCond, Else, ElCond, and EndIf are allowed
using CondDirective = Directive;
using MinimizeCondCtx = std::vector<std::variant<std::string, CondDirective>>;

std::string minimizeCondToStr(MinimizeCondCtx& mcCtx);

}