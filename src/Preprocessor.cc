#include "Preprocessor.hpp"
#include "OptProcessor.hpp"
#include "FileOp.hpp"
#include "Directive.hpp"
#include <algorithm>
#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace {

// Return the position one-past the closing character
template <char open, char close> void balance(std::string_view str, size_t& pos) {
  size_t nest{1};
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

PreprocessRes preprocess(const Opts& opts, File& file) {
  PreprocessRes res;
  bool lookForMain{file.type == FileType::UnpairedSrc};
  bool SOFComment{opts.SOFComments};
  IncludeGuardCtx ctx{file.type, opts.includeGuard};
  bool whitespaceAfterNewline{true};
  size_t i{};
  size_t totalLen{file.content.length()};
  size_t start;
  size_t end;
  size_t len;
  Directive directive;
  auto rmDirective = [&] {
    std::copy(file.content.begin() + end, file.content.end(),
      file.content.begin() + start);
    i = start;
    totalLen -= len;
  };
  auto emplaceDirective = [&] {
    res.directives.emplace_back(std::move(directive));
  };
  while(i < totalLen) {
    if(SOFComment && file.content[i] != '/' && file.content[i] != '\n') {
      SOFComment = false;
    }
    switch(file.content[i]) {
    
    // Comments
    case '/':
      ++i;
      if(file.content[i] == '/') {
        while(i < totalLen && file.content[i] != '\n') ++i;
        if(SOFComment) res.prefixNewlineCnt = 1;
      }
      else if(file.content[i] == '*') {
        ++i;
        while(!(file.content[i - 1] == '*' && file.content[i] == '/')) ++i;
        if(SOFComment) res.prefixNewlineCnt = 2;
      }
      if(SOFComment) res.insertionPos = i + 1;
      break;

    // Char/Int literal
    case '\'':
      ++i;

      // Int literals
      if(isDigit(file.content[i]) && isDigit(file.content[i - 2]) &&
        file.content[i - 3] != 'u') break;

      // Char literals
      while(file.content[i] != '\'') i += (file.content[i] == '\\') + 1;
      break;

    // String literals
    case '"':
      ++i;

      // Raw
      if(file.content[i - 2] == 'R') {
        start = i;
        while(file.content[i] != '(') ++i;
        const size_t delimSize{i - start};
        balance<'(',')'>(file.content, ++i);
        i += delimSize;
      }
      else while(file.content[i] != '"') i += (file.content[i] == '\\') + 1;
      break;

    // Preprocessor directives
    case '#':
      if(!whitespaceAfterNewline) break;
      start = i;
      while(i < totalLen && (file.content[i] != '\n' || file.content[i - 1] == '\\')) ++i;
      
      // Get the \n if available
      end = i + (i < totalLen);
      len = end - start;
      directive = {file.content.substr(start, len), ctx, opts};
      switch(directive.type) {
      case DirectiveType::Define:
        if(std::holds_alternative<IncludeGuard>(directive.extraInfo)) {
          ctx.state = IncludeGuardState::GotDefine;
          if(opts.transitional) emplaceDirective();
          rmDirective();
        }
        else emplaceDirective();
        break;
      case DirectiveType::IfCond:
        if(ctx.state == IncludeGuardState::GotDefine) ctx.counter++;
        else if(std::holds_alternative<IncludeGuard>(directive.extraInfo)) {
          ctx.state = IncludeGuardState::GotIfndef;
          ctx.counter = 1;
          if(opts.transitional) emplaceDirective();
          rmDirective();
          break;
        }
        emplaceDirective();
        break;
      case DirectiveType::EndIf:
        if(ctx.state == IncludeGuardState::GotDefine) {
          ctx.counter--;
          if(ctx.counter == 0) {
            ctx.state = IncludeGuardState::GotEndIf;
            if(!opts.transitional) rmDirective();
            break;
          }
        }
        [[fallthrough]];
      case DirectiveType::Else:
      case DirectiveType::ElCond:
      case DirectiveType::Undef:
        emplaceDirective();
        break;
      case DirectiveType::PragmaOnce:
        if(opts.transitional) emplaceDirective();
        rmDirective();
        break;
      case DirectiveType::Include:
        emplaceDirective();
        rmDirective();
        break;
      default:;
      }
      continue;
    case '\n':
      whitespaceAfterNewline = true;
      break;
    default:
      whitespaceAfterNewline = isSpace(file.content[i]);

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
    ++i;
  }

  // In case the file only contains commments
  if(SOFComment) {
    res.prefixNewlineCnt = 2;
    res.insertionPos = i;
  }
  file.content.resize(totalLen);
  return res;
}