#include "CondMinimizer.hpp"
#include "Directive.hpp"
#include <string>
#include <variant>

std::string minimizeCondToStr(MinimizeCondCtx& mcCtx) {
  std::string rtn;
  for(const std::variant<std::string, Directive>& i : mcCtx) {
    switch(i.index()) {
    case 0:
      rtn += std::get<std::string>(i);
      break;
    case 1:
      rtn += std::get<Directive>(i).str;
    }
  }
  return rtn;
}