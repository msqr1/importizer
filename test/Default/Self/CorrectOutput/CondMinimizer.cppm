module;
#include <string>
#include <vector>
#include <variant>
export module CondMinimizer;
import Directive;


using MinimizeCondCtx = std::vector<std::variant<std::string, Directive>>;

std::string minimizeCondToStr(MinimizeCondCtx& mcCtx);