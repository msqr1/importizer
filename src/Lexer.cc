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

// FIXME: Make it also detect if a main() is available
// If you're a sane person you wouldn't write the main function like this:
// int/*comment*/main/*comment*/(, right? Cuz it won't work.
LexResult lex(File& file) {
  LexResult lexRes;
  bool lookForMain{file.type == FileType::PairedSrc || file.type == FileType::UnpairedSrc};
  bool whitespaceAfterNewline{true};
  size_t i{};
  std::string& code{file.content};
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
        
        // Get the \n  if available
        const size_t end{i + (i < codeLen)};
        const size_t len{end - start};
        std::string directive{code.substr(start, len)};
        DirectiveType type{directiveTypeFromStr(directive)};
        switch(type) {
        case DirectiveType::Other:
          continue;
        case DirectiveType::PragmaOnce:
        case DirectiveType::Include:
        case DirectiveType::Define:
        case DirectiveType::Undef:
          std::copy(code.begin() + end, code.end(), code.begin() + start);
          i = start;
          codeLen -= len;
          [[fallthrough]];
        default:
          lexRes.directives.emplace_back(type, std::move(directive));
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
  return lexRes;
}