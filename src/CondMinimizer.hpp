#include "Directive.hpp"
#include <string>
#include <vector>
#include <variant>

using MinimizeCondCtx = std::vector<std::variant<std::string, Directive>>;

std::string minimizeCondToStr(MinimizeCondCtx& mcCtx);