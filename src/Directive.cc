#include "Directive.hpp"
#include "Base.hpp"
#include "Regex.hpp"
#include "ArgProcessor.hpp"
#include "FileOp.hpp"
#include <string>
#include <vector>

namespace {

template <char open, char close>
void balance(std::string_view str, size_t& pos) {
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

// Doesn't work when comments are inside the directive
// Such as #/*comment*/include/*comment*/<iostream>
// because this skips over space until it sees a character
// Not sure why anyone would do this.
Directive::Type directiveTypeFromStr(std::string_view val) {
  val.remove_prefix(1);
  size_t startDirective{};
  while(val[startDirective] == ' ') startDirective++;
  size_t endDirective{startDirective};
  while(val[endDirective] != ' ' && val[endDirective] != '\n' 
    && val[endDirective] != '/') endDirective++;
  std::string_view directive{val.substr(startDirective, endDirective - startDirective)};
  if(directive == "define") return Directive::Type::Define;
  if(directive == "undef") return Directive::Type::Undef;
  std::string_view first2Chars{directive.substr(0, 2)};
  if(first2Chars == "if" || first2Chars == "el") return Directive::Type::Condition;
  if(directive == "endif") return Directive::Type::EndIf;
  if(directive == "include") return Directive::Type::Include;
  if(directive == "pragma") {
    startDirective = endDirective;
    while(val[startDirective] == ' ') startDirective++;
    size_t endDirective = startDirective;
    while(val[endDirective] != ' ' && val[endDirective] != '\n' 
    && val[endDirective] != '/') endDirective++;
    directive = val.substr(startDirective, endDirective - startDirective);
    if(directive == "once") return Directive::Type::PragmaOnce;
  }
  return Directive::Type::Other;
}

}

Directive::Directive(Type type_, std::string_view val_): type{type_}, val{val_} {
  if(val_.back() != '\n') val += '\n';
}

std::vector<Directive> lexDirectives(std::string& code) {
  std::vector<Directive> directives;
  bool whitespaceAfterNewline{true};
  for(size_t i{}; i < code.length(); i++) {

    // Skip comments
    if(code[i] == '/') {
      i++;
      if(code[i] == '/') {
        while(i < code.length() && code[i] != '\n') i++;
      }
      else if(code[i] == '*') {
        i++;
        while(!(code[i - 1] == '*' && code[i] == '/')) i++;
      }
    }

    // Skip string literals
    else if(code[i] == '"') {
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
    }
    if(whitespaceAfterNewline && code[i] == '#') {
      const size_t start{i};
      while(i < code.length() && code[i] != '\n') i++;

      // Get the \n if available
      const size_t end{i + (i < code.length())};
      const std::string directive{code.substr(start, end - start)};
      Directive::Type type{directiveTypeFromStr(directive)};
      switch(type) {
      case Directive::Type::Other:
        continue;
      case Directive::Type::PragmaOnce:
        std::copy(code.begin() + end, code.end(), code.begin() + start);
        code.resize(code.length() - (end - start));
        i -= end - start;
      default:
        directives.emplace_back(type, code.substr(start, end - start));
      }
    }
    else if(code[i] == '\n') whitespaceAfterNewline = true;
    else whitespaceAfterNewline = isspace(code[i]);
  }
  return directives;
};