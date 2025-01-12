#include "Directive.hpp"
#include "Base.hpp"
#include "Regex.hpp"
#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

IncludeInfo::IncludeInfo(size_t startOffset, bool isAngle, std::string_view includeStr): 
  isAngle{isAngle}, startOffset{startOffset}, includeStr{includeStr} {}

Directive::Directive(std::string&& str_, const IncludeGuardCtx& ctx) : str{str_} {
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
    if(ctx.state == IncludeGuardState::Looking && 
      ctx.pat.match(getWord(1 + directive.length(), str))) {
      extraInfo.emplace<IncludeGuard>();
    }
    break;
  case DirectiveType::Define: {
    if(ctx.state == IncludeGuardState::GotIfndef &&
      ctx.pat.match(getWord(1 + directive.length(), str))) {
      extraInfo.emplace<IncludeGuard>();
    }
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
    extraInfo.emplace<IncludeInfo>(start, isAngle, 
      std::string_view(str.c_str() + start, end - start));
    break;
  }
  default:
  }
}

Directive::Directive(Directive&& other) {
  type = other.type;
  extraInfo = std::move(other.extraInfo);
  switch(other.extraInfo.index()) {
  case 1: {
    IncludeInfo& includeInfo{std::get<IncludeInfo>(extraInfo)};
    size_t len{includeInfo.includeStr.length()};
    str = std::move(other.str);
    includeInfo.includeStr = std::string_view(str.c_str() + includeInfo.startOffset, len);
    break;
  }
  default:
    str = std::move(other.str);
  }
}