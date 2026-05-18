module;
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <string>
#include <string_view>
#include <optional>
#include <utility>
#include <variant>
#include <vector>
module Preprocessor;
import OptProcessor;
import Regex;
import FileOp;
import Directive;


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
  Continue,
  Emplace,
  EmplaceRemove,
  Remove,
};

} // namespace

std::vector<Directive> preprocess(const std::optional<TransitionalOpts>& transitionalOpts, 
  File& file, const re::Pattern& includeGuardPat) {
  std::vector<Directive> directives;
  bool lookForMain{file.type == FileType::PairedSrc || file.type == FileType::UnpairedSrc};
  IncludeGuardCtx ctx{file.type, includeGuardPat};
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
    case '\'':
      i++;
      while(code[i] != '\'') i += (code[i] == '\\') + 1;
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
      else while(code[i] != '"') i += (code[i] == '\\') + 1;
      break;
    case '\n':
      whitespaceAfterNewline = true;
      break;
    default:
      if(whitespaceAfterNewline && code[i] == '#') {
        const size_t start{i};
        while(i < codeLen && (code[i] != '\n' || code[i - 1] == '\\')) i++;
        
        // Get the \n if available
        const size_t end{i + (i < codeLen)};
        const size_t len{end - start};
        DirectiveAction directiveAction;
        Directive directive{code.substr(start, end - start), ctx};
        switch(directive.type) {
        case DirectiveType::Define:
          if(std::holds_alternative<IncludeGuard>(directive.extraInfo)) {
            ctx.state = IncludeGuardState::GotDefine;
            directiveAction = transitionalOpts ?
              DirectiveAction::EmplaceRemove : DirectiveAction::Remove;
          }
          else directiveAction = DirectiveAction::Emplace;
          break;
        case DirectiveType::IfCond:
          if(ctx.state == IncludeGuardState::GotDefine) ctx.counter++;
          else if(std::holds_alternative<IncludeGuard>(directive.extraInfo)) {
            ctx.state = IncludeGuardState::GotIfndef;
            ctx.counter = 1;
            directiveAction = transitionalOpts ?
              DirectiveAction::EmplaceRemove : DirectiveAction::Remove;
            break;
          }
          directiveAction = DirectiveAction::Emplace;
          break;
        case DirectiveType::EndIf:
          if(ctx.state == IncludeGuardState::GotDefine) {
            ctx.counter--;
            if(ctx.counter == 0) {
              ctx.state = IncludeGuardState::GotEndif;
              directiveAction = transitionalOpts ?
                DirectiveAction::Continue : DirectiveAction::Remove;
              break;
            }
          }
          [[fallthrough]];
        case DirectiveType::Else:
        case DirectiveType::ElCond:
        case DirectiveType::Undef:
          directiveAction = DirectiveAction::Emplace;
          break;
        case DirectiveType::PragmaOnce:
          directiveAction = transitionalOpts ?
            DirectiveAction::EmplaceRemove : DirectiveAction::Remove;
          break;
        case DirectiveType::Include:
          directiveAction = DirectiveAction::EmplaceRemove;
          break;
        case DirectiveType::Other:
          directiveAction = DirectiveAction::Continue;
          break;
        }
        switch(directiveAction) {
        case DirectiveAction::Emplace:
          directives.emplace_back(std::move(directive));
          break;
        case DirectiveAction::EmplaceRemove:
          directives.emplace_back(std::move(directive));
          [[fallthrough]];
        case DirectiveAction::Remove:
          std::copy(code.begin() + end, code.end(), code.begin() + start);
          i = start;
          codeLen -= len;
          break;
        case DirectiveAction::Continue:;
        }
        continue;
      }
      else whitespaceAfterNewline = std::isspace(code[i]);

      // If you're a sane person you wouldn't write the main function like this:
      // int/*comment*/main/*comment*/(, right? Cuz it won't work.
      if(lookForMain && std::string_view(code.c_str() + i, 3) == "int") {
        i = code.find_first_not_of(" \n\t", i + 3);
        if(std::string_view(code.c_str() + i, 4) != "main") break;
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
  return directives;
}