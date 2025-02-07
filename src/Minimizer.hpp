#include "Directive.hpp"
#include <string>
#include <vector>
#include <variant>

using MinimizeCtx = std::vector<std::variant<std::string, Directive>>;

std::string minimizeToStr(MinimizeCtx& mCtx);