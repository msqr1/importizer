#include "Directive.hpp"
#include "Base.hpp"
#include "Regex.hpp"

IncludeInfo::IncludeInfo(size_t startOffset, bool isAngle, std::string_view includeStr): 
  isAngle{isAngle}, startOffset{startOffset}, includeStr{includeStr} {}

GuardInfo::GuardInfo(size_t startOffset, std::string_view identifier):
  startOffset{startOffset}, identifier{identifier} {}

IncludeGuardCtx::IncludeGuardCtx(bool lookFor, const std::optional<re::Pattern>& pat):
  state{lookFor ? IncludeGuardState::Looking : IncludeGuardState::NotLooking}, pat{pat},
  counter{1} {}

Directive::Directive(std::string&& str_) : str{str_} {
  if(str.back() != '\n') str += '\n';
  auto getWord = [](size_t start, std::string_view str) {
    while(str[start] == ' ') start++;
    size_t end{start};
    while(str[end] != ' ' && str[end] != '\n' 
      && str[end] != '/') end++;
    return str.substr(start, end - start);
  };
  std::string_view directive{getWord(1, str)};
  std::string_view first2Chars{directive.substr(0, 2)};
  if(directive == "define") type = DirectiveType::Define;
  else if(directive == "undef") type = DirectiveType::Undef;
  else if(directive == "include") type = DirectiveType::Include;
  else if(directive == "ifndef") type = DirectiveType::Ifndef;
  else if(directive == "endif") type = DirectiveType::EndIf;  
  else if(first2Chars == "if") type = DirectiveType::IfCond;
  else if(first2Chars == "el") type = DirectiveType::ElCond;
  else if(directive == "pragma" && getWord(1 + directive.length(), str) == "once") {
    type = DirectiveType::PragmaOnce;
  }
  else type = DirectiveType::Other;
  switch(type) {
  case DirectiveType::Ifndef:
  case DirectiveType::Define: {
    size_t start{1 + directive.length()};
    while(str[start] == ' ') start++;
    size_t end{start};
    while(str[end] != ' ' && str[end] != '\n' 
      && str[end] != '/') end++;
    info.emplace<GuardInfo>(start, std::string_view(str.c_str() + start, end - start));
    break;
  }
  case DirectiveType::Include: {
    size_t start{str.find('<', 1 + directive.length())};
    size_t end; 
    bool isAngle{start != notFound};
    if(isAngle) {
      start++;
      end = str.find('>', start);
    }
    else {
      start = str.find('"') + 1;
      end = str.find('"', start);
    }
    info.emplace<IncludeInfo>(start, isAngle, 
      std::string_view(str.c_str() + start, end - start));
    break;
  }
  default:
  }
}

Directive::Directive(Directive&& other) {
  type = other.type;
  info = std::move(other.info);
  switch(other.info.index()) {
  case 1: {
    IncludeInfo& includeInfo{std::get<IncludeInfo>(info)};
    size_t len{includeInfo.includeStr.length()};
    str = std::move(other.str);
    includeInfo.includeStr = std::string_view(str.c_str() + includeInfo.startOffset, len);
    break;
  }
  case 2: {
    GuardInfo& guardInfo{std::get<GuardInfo>(info)};
    size_t len{guardInfo.identifier.length()};
    str = std::move(other.str);
    guardInfo.identifier = std::string_view(str.c_str() + guardInfo.startOffset, len);
    break;
  }
  }
}

DirectiveAction getDirectiveAction(const Directive& directive, IncludeGuardCtx& ctx) {
  switch(directive.type) {
  case DirectiveType::Ifndef:
    if(ctx.state == IncludeGuardState::Looking && 
      ctx.pat->match(std::get<GuardInfo>(directive.info).identifier)) {
      ctx.state = IncludeGuardState::GotIfndef;
      return DirectiveAction::Remove;
    }
    return DirectiveAction::EmplaceRemove;
  case DirectiveType::Define:
    if(ctx.state == IncludeGuardState::GotIfndef &&
      ctx.pat->match(std::get<GuardInfo>(directive.info).identifier)) {
      ctx.state = IncludeGuardState::GotDefine;
      return DirectiveAction::Remove; 
    }
    return DirectiveAction::EmplaceRemove;
  case DirectiveType::IfCond:
    if(ctx.state == IncludeGuardState::GotDefine) ctx.counter++;
    return DirectiveAction::EmplaceRemove;
  case DirectiveType::EndIf:
    if(ctx.state == IncludeGuardState::GotDefine) {
      ctx.counter--;
      if(ctx.counter == 0) return DirectiveAction::Remove;
    }
    return DirectiveAction::EmplaceRemove;
  case DirectiveType::Include:
  case DirectiveType::ElCond:
  case DirectiveType::Undef:
    return DirectiveAction::EmplaceRemove;
  case DirectiveType::PragmaOnce:
  case DirectiveType::Other:
    return DirectiveAction::Remove;
  }
}