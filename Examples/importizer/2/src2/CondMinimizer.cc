module;
#include <cstddef>
#include <optional>
#include <string>
#include <variant>
module CondMinimizer;
import Directive;


std::optional<size_t> getIfSkip(const MinimizeCondCtx& mcCtx, size_t currentIdx) {
  currentIdx++;
  for(; currentIdx < mcCtx.size(); currentIdx++) {
    if(std::holds_alternative<CondDirective>(mcCtx[currentIdx])) {
      if(std::get<CondDirective>(mcCtx[currentIdx]).type == DirectiveType::EndIf) {
        return currentIdx;
      }
    }
    else return std::nullopt;
  }
  return std::nullopt;
}
std::string minimizeCondToStr(MinimizeCondCtx& mcCtx) {
  std::string rtn;
  for(size_t i{}; i < mcCtx.size(); i++) switch(mcCtx[i].index()) {
  case 0: // std::string
    rtn += std::get<std::string>(mcCtx[i]);
    break;
  case 1: // CondDirective
    const CondDirective& current{std::get<CondDirective>(mcCtx[i])};
    if(i < mcCtx.size() - 1 && std::holds_alternative<CondDirective>(mcCtx[i + 1])) {
      const CondDirective& next{std::get<CondDirective>(mcCtx[i + 1])};
      switch(current.type) {
      case DirectiveType::IfCond:
        if(std::optional<size_t> ifSkip{getIfSkip(mcCtx, i)}) i = *ifSkip;
        continue;
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