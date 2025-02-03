#include "CondMinimizer.hpp"
#include "Directive.hpp"
#include <cstddef>
#include <optional>
#include <string>
#include <variant>

std::optional<size_t> getIfSkip(const MinimizeCondCtx& mcCtx, size_t currentIdx) {
  currentIdx++;
  size_t nest{1};
  for(; currentIdx < mcCtx.size(); currentIdx++) {
    if(std::holds_alternative<CondDirective>(mcCtx[currentIdx])) {
      DirectiveType type{std::get<CondDirective>(mcCtx[currentIdx]).type};
      if(type == DirectiveType::IfCond) nest++;
      else if(type == DirectiveType::EndIf) {
        nest--;
        if(nest == 0) return currentIdx;
      }
    }
    else return std::nullopt;
  }
  return std::nullopt;
}
std::string minimizeCondToStr(MinimizeCondCtx& mcCtx) {
  std::string rtn;
  std::optional<size_t> ifSkip;
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
        if((ifSkip = getIfSkip(mcCtx, i))) {
          i = *ifSkip;
          continue;
        }
        break;
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