#include "Lexer.hpp"
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
DirectiveType directiveTypeFromStr(std::string_view str) {
  str.remove_prefix(1);
  size_t startDirective{};
  while(str[startDirective] == ' ') startDirective++;
  size_t endDirective{startDirective};
  while(str[endDirective] != ' ' && str[endDirective] != '\n' 
    && str[endDirective] != '/') endDirective++;
  std::string_view directive{str.substr(startDirective, endDirective - startDirective)};
  if(directive == "define") return DirectiveType::Define;
  if(directive == "undef") return DirectiveType::Undef;
  std::string_view first2Chars{directive.substr(0, 2)};
  if(first2Chars == "if" || first2Chars == "el") return DirectiveType::Condition;
  if(directive == "endif") return DirectiveType::EndIf;
  if(directive == "include") return DirectiveType::Include;
  if(directive == "pragma") {
    startDirective = endDirective;
    while(str[startDirective] == ' ') startDirective++;
    size_t endDirective = startDirective;
    while(str[endDirective] != ' ' && str[endDirective] != '\n' 
    && str[endDirective] != '/') endDirective++;
    directive = str.substr(startDirective, endDirective - startDirective);
    if(directive == "once") return DirectiveType::PragmaOnce;
  }
  return DirectiveType::Other;
}

}

Directive::Directive(DirectiveType type_, std::string_view str_): type{type_}, str{str_} {
  if(str_.back() != '\n') str += '\n';
}

// FIXME: It's still behaving like string erase when seeing an include.
// Fix this so that it changes to string remove()
// FIXME: Make it also detect if a main() is available
LexResult lex(std::string& code) {
  logIfVerbose("Lexing...");
  LexResult lexRes;
  bool whitespaceAfterNewline{true};
  size_t i{};
  size_t codeLen{code.length()};
  while(i < codeLen) {

    // Skip comments
    if(code[i] == '/') {
      i++;
      if(code[i] == '/') {
        while(i < codeLen && code[i] != '\n') i++;
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
      while(i < codeLen && code[i] != '\n') i++;
      
      // Get the \n if available
      const size_t end{i + (i < codeLen)};
      std::string directive{code.substr(start, end - start)};
      DirectiveType type{directiveTypeFromStr(directive)};
      switch(type) {
      case DirectiveType::Other:
        continue;
      case DirectiveType::PragmaOnce:
      case DirectiveType::Include:
        std::copy(code.begin() + end, code.end(), code.begin() + start);
        i = start;
        codeLen -= end - start;
        [[fallthrough]];
      default:
        lexRes.directives.emplace_back(type, std::move(directive));
      }
      continue;
    }
    else if(code[i] == '\n') whitespaceAfterNewline = true;
    else whitespaceAfterNewline = isspace(code[i]);
    i++;
  }
  code.resize(codeLen);
  return lexRes;
};