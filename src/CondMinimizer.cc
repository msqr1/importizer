#include "CondMinimizer.hpp"
#include "Directive.hpp"
#include <cstddef>
#include <string>
#include <variant>

std::string minimizeCondToStr(MinimizeCondCtx& mcCtx) {
  std::string rtn;
  for(size_t i{}; i < mcCtx.size(); i++) switch(mcCtx[i].index()) {
  case 0:
    rtn += std::get<std::string>(mcCtx[i]);
    break;
  case 1:
    const Directive& current{std::get<Directive>(mcCtx[i])};
    if(i < mcCtx.size() - 1 && std::holds_alternative<Directive>(mcCtx[i + 1])) {
      const Directive& next{std::get<Directive>(mcCtx[i + 1])};
      switch(current.type) {
      case DirectiveType::IfCond:
        if(next.type == DirectiveType::EndIf) {
          i++;
          continue;
        }
      case DirectiveType::Else:
        if(next.type == DirectiveType::EndIf) {
          i++;
          rtn += next.str;
          continue;
        }
        break;
      case DirectiveType::ElCond:
        switch(next.type) {
        case DirectiveType::ElCond:
        case DirectiveType::EndIf:
          rtn += next.str;
          i++;
          continue;
        case DirectiveType::Else:      
          continue;
        default:;
        }
        break;
      default:;
      }
    }
    rtn += current.str;
  }
  return rtn; 
}