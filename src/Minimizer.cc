#include "Minimizer.hpp"
#include "Directive.hpp"
#include <cstdint>
#include <optional>
#include <string>
#include <variant>

std::optional<uintmax_t> getIfElSkip(const MinimizeCtx& mCtx, uintmax_t currentIdx,
  DirectiveType currentType) {
  currentIdx++;
  uintmax_t nest{1};
  for(; currentIdx < mCtx.size(); currentIdx++) {
    if(!std::holds_alternative<Directive>(mCtx[currentIdx])) return std::nullopt;
    DirectiveType type{std::get<Directive>(mCtx[currentIdx]).type};
    switch(type) {
    case DirectiveType::IfCond:
      nest++;
      break;
    case DirectiveType::EndIf:
      nest--;
      if(nest == 0) return currentIdx - (currentType == DirectiveType::ElCond);
      break;
    case DirectiveType::ElCond:
    case DirectiveType::Else:
      break;
    default:
      return std::nullopt;
    }
  }
  return std::nullopt;
}
std::string minimizeToStr(const MinimizeCtx& mCtx) {
  std::string rtn;
  std::optional<uintmax_t> ifSkip;
  for(uintmax_t i{}; i < mCtx.size(); i++) switch(mCtx[i].index()) {
  case 0: // std::string
    rtn += std::get<std::string>(mCtx[i]);
    break;
  case 1: // Directive
    const Directive& current{std::get<Directive>(mCtx[i])};
    if(i < mCtx.size() - 1 && std::holds_alternative<Directive>(mCtx[i + 1])) {
      const Directive& next{std::get<Directive>(mCtx[i + 1])};
      switch(current.type) {
      case DirectiveType::Define:
        if(next.type == DirectiveType::Undef) {
          i++;
          continue;
        }
        break;
      case DirectiveType::IfCond:
      case DirectiveType::ElCond:
        if((ifSkip = getIfElSkip(mCtx, i, current.type))) {
          i = *ifSkip;
          continue;
        }
        break;
      case DirectiveType::Else:
        if(next.type == DirectiveType::EndIf) continue;
        break;
      default:;
      }
    }
    rtn += current.str;
  }
  return rtn;
}