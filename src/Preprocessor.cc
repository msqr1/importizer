#include "Preprocessor.hpp"
#include "Base.hpp"
#include "Regex.hpp"
#include "ArgProcessor.hpp"
#include "FileOp.hpp"
#include <coroutine>
#include <string>
#include <vector>

namespace {

template <char open, char close> void balance(std::string_view str, size_t& pos) {
  int nest{1};
  do {
    switch(str[pos]) {
    case open:
      nest++;
      break;
    case close:
      nest--;
    }
    ++pos;
  } while(nest); 
}

enum class DirectiveAction : char {
  Ignore,
  EmplaceRemove,
  Remove,
};
enum class IncludeGuardState : char {
  NotLooking,
  Looking,
  GotIfndef,
  GotDefine,
  GotEndif
};
struct IncludeGuardCtx {
  IncludeGuardState state;
  const std::optional<re::Pattern>& pat;
  size_t counter;
  IncludeGuardCtx(bool lookFor, const std::optional<re::Pattern>& pat):
    state{lookFor ? IncludeGuardState::Looking : IncludeGuardState::NotLooking}, 
    pat{pat}, counter{1} {}
};
DirectiveAction handleDirective(const Directive& directive, IncludeGuardCtx& ctx) {
  switch(directive.type) {
  case DirectiveType::Ifndef:
    if(ctx.state == IncludeGuardState::Looking && 
      ctx.pat->match(std::get<std::string_view>(directive.info))) {
      ctx.state = IncludeGuardState::GotIfndef;
      return DirectiveAction::Remove;    
    }
    return DirectiveAction::EmplaceRemove;
  case DirectiveType::Define:
    if(ctx.state == IncludeGuardState::GotIfndef &&
      ctx.pat->match(std::get<std::string_view>(directive.info))) {
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
  case DirectiveType::ElCond:
  case DirectiveType::Include:
  case DirectiveType::Undef:
    return DirectiveAction::EmplaceRemove;
  case DirectiveType::PragmaOnce:
  case DirectiveType::Other:
    return DirectiveAction::Remove;
  }
}

}

IncludeInfo::IncludeInfo(bool isAngle, std::string_view includeStr): isAngle{isAngle},
  includeStr{includeStr} {}
Directive::Directive(std::string&& str_) : str{str_} {
  if(str.back() != '\n') str += '\n';
  
  auto getWord = [](size_t start, std::string_view str){
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
  case DirectiveType::Define:
    info = getWord(1 + directive.length(), str);
    break;
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
    info.emplace<IncludeInfo>(isAngle, std::string_view(str.c_str() + start, end - start));
    break;
  }
  default:
  }
}
Directive::Directive(Directive&& other) {
  type = other.type;
  str = std::move(other.str);
  info = std::move(other.info);
}

// If you're a sane person you wouldn't write the main function like this:
// int/*comment*/main/*comment*/(, right? Cuz it won't work.
PreprocessResult preprocess(File& file, 
  const std::optional<re::Pattern>& maybeIncludeGuardPat) {
  PreprocessResult preprocessRes;
  bool lookForMain{file.type == FileType::PairedSrc || file.type == FileType::UnpairedSrc};
  IncludeGuardCtx ctx{maybeIncludeGuardPat && file.type == FileType::Hdr, 
    maybeIncludeGuardPat};
  bool whitespaceAfterNewline{true};
  std::string& code{file.content};
  size_t i{};
  size_t codeLen{code.length()};
  while(i < codeLen) {
    switch(code[i]) {
    case '/':
      i++;
      if(code[i] == '/') {
        while(i < codeLen && code[i] != '\n') i++;
      }
      else if(code[i] == '*') {
        i++;
        while(!(code[i - 1] == '*' && code[i] == '/')) i++;
      }
      break;
    case '"':
      i++;

      // Raw string literal
      if(code[i - 2] == 'R') {
        const size_t start{i};
        while(code[i] != '(') i++;
        const size_t delimSize{i - start};
        balance<'(',')'>(code, i);
        i += delimSize;
      }
      else while(code[i] == '"') i++;
      break;
    case '\n':
      whitespaceAfterNewline = true;
      break;
    default:
      if(whitespaceAfterNewline && code[i] == '#') {
        const size_t start{i};
        while(i < codeLen && code[i] != '\n') i++;
        
        // Get the \n if available
        const size_t end{i + (i < codeLen)};
        const size_t len{end - start};
        Directive directive{code.substr(start, end - start)};
        switch(handleDirective(directive, ctx)) {
        case DirectiveAction::EmplaceRemove:
          preprocessRes.directives.emplace_back(std::move(directive));
          [[fallthrough]];
        case DirectiveAction::Remove:
          std::copy(code.begin() + end, code.end(), code.begin() + start);
          i = start;
          codeLen -= len;
          break;
        case DirectiveAction::Ignore:
        }
        continue;
      }
      else whitespaceAfterNewline = isspace(code[i]);
      if(lookForMain && code[i] == 'i' && 
        code[i + 1] == 'n' && code[i + 2] == 't') {
        i = code.find_first_not_of(" \n\t", i + 3);
        if(!(code[i] == 'm' && code[i + 1] == 'a' && code[i + 2] == 'i' && 
          code[i + 3] == 'n')) break;
        i = code.find_first_not_of(" \n\t", i + 4);
        if(code[i] == '(') {
          lookForMain = false;
          file.type = FileType::SrcWithMain;
        }
      }
    }
    i++;
  }
  code.resize(codeLen);
  return preprocessRes;
}