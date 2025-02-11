#include "Preprocessor.hpp"
#include "OptProcessor.hpp"
#include "FileOp.hpp"
#include "Directive.hpp"
#include <algorithm>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace {

// Return the position one-past the closing character
template <char open, char close> void balance(std::string_view str, uintmax_t& pos) {
  uintmax_t nest{1};
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
bool isDigit(char c) {
  for(char n : "0123456789") {
    if(c == n) return true;
  }
  return false;
}
bool isSpace(char c) {
  for(char n : " \t\v\f") {
    if(c == n) return true;
  }
  return false;
}

} // namespace

std::vector<Directive> preprocess(const Opts& opts, File& file) {
  std::vector<Directive> directives;
  bool lookForMain{file.type == FileType::UnpairedSrc};
  IncludeGuardCtx ctx{file.type, opts.includeGuardPat};
  bool whitespaceAfterNewline{true};
  uintmax_t i{};
  uintmax_t codeLen{file.content.length()};
  uintmax_t start;
  uintmax_t end;
  uintmax_t len;
  auto rmDirective = [&] {
    std::copy(file.content.begin() + end, file.content.end(),
      file.content.begin() + start);
    i = start;
    codeLen -= len;
  };
  while(i < codeLen) {
    switch(file.content[i]) {
    
    // Comments
    case '/':
      i++;
      if(file.content[i] == '/') while(i < codeLen && file.content[i] != '\n') i++;
      else if(file.content[i] == '*') {
        i++;
        while(!(file.content[i - 1] == '*' && file.content[i] == '/')) i++;
      }
      break;

    // Character/Integer literal
    case '\'':
      i++;

      // Integer literals
      if(isDigit(file.content[i]) && isDigit(file.content[i - 2]) && file.content[i - 3] != 'u') break;

      // Character literals
      while(file.content[i] != '\'') i += (file.content[i] == '\\') + 1;
      break;

    // String literals
    case '"':
      i++;

      // Raw
      if(file.content[i - 2] == 'R') {
        const uintmax_t start{i};
        while(file.content[i] != '(') i++;
        const uintmax_t delimSize{i - start};
        balance<'(',')'>(file.content, ++i);
        i += delimSize;
      }
      else while(file.content[i] != '"') i += (file.content[i] == '\\') + 1;
      break;
    case '\n':
      whitespaceAfterNewline = true;
      break;
    default:

      // Preprocessor directive
      if(whitespaceAfterNewline && file.content[i] == '#') {
        start = i;
        while(i < codeLen && (file.content[i] != '\n' || file.content[i - 1] == '\\')) i++;
        
        // Get the \n if available
        end = i + (i < codeLen);
        len = end - start;
        Directive directive{file.content.substr(start, len), ctx};
        /*if(file.type == FileType::IgnoredHdr) {
          
        }
        else */
        switch(directive.type) {
        case DirectiveType::Define:
          if(std::holds_alternative<IncludeGuard>(directive.extraInfo)) {
            ctx.state = IncludeGuardState::GotDefine;
            if(opts.transitionalOpts) directives.emplace_back(std::move(directive));
            rmDirective();
          }
          else directives.emplace_back(std::move(directive));
          break;
        case DirectiveType::IfCond:
          if(ctx.state == IncludeGuardState::GotDefine) ctx.counter++;
          else if(std::holds_alternative<IncludeGuard>(directive.extraInfo)) {
            ctx.state = IncludeGuardState::GotIfndef;
            ctx.counter = 1;
            if(opts.transitionalOpts) directives.emplace_back(std::move(directive));
            rmDirective();
            break;
          }
          directives.emplace_back(std::move(directive));
          break;
        case DirectiveType::EndIf:
          if(ctx.state == IncludeGuardState::GotDefine) {
            ctx.counter--;
            if(ctx.counter == 0) {
              ctx.state = IncludeGuardState::GotEndif;
              if(!opts.transitionalOpts) rmDirective();
              break;
            }
          }
          [[fallthrough]];
        case DirectiveType::Else:
        case DirectiveType::ElCond:
        case DirectiveType::Undef:
          directives.emplace_back(std::move(directive));
          break;
        case DirectiveType::PragmaOnce:
          if(opts.transitionalOpts) directives.emplace_back(std::move(directive));
          rmDirective();
          break;
        case DirectiveType::Include:
          directives.emplace_back(std::move(directive));
          rmDirective();
          break;
        case DirectiveType::Other:;
        }
        continue;
      }
      else whitespaceAfterNewline = isSpace(file.content[i]);

      // If you're a sane person you wouldn't write the main function like this:
      // int/*comment*/main/*comment*/(, right? Cuz it won't work.
      if(lookForMain && std::string_view(file.content.c_str() + i, 3) == "int") {
        i = file.content.find_first_not_of(" \n\t", i + 3);
        if(std::string_view(file.content.c_str() + i, 4) != "main") break;
        i = file.content.find_first_not_of(" \n\t", i + 4);
        if(file.content[i] == '(') {
          lookForMain = false;
          file.type = FileType::SrcWithMain;
        }
      }
    }
    i++;
  }
  file.content.resize(codeLen);
  return directives;
}