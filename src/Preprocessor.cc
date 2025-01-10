#include "Preprocessor.hpp"
#include "Base.hpp"
#include "Regex.hpp"
#include "FileOp.hpp"
#include "Directive.hpp"
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

}

// If you're a sane person you wouldn't write the main function like this:
// int/*comment*/main/*comment*/(, right? Cuz it won't work.
PreprocessResult preprocess(File& file, 
  const re::Pattern& includeGuardPat) {
  logIfVerbose("Preprocessing...");
  PreprocessResult preprocessRes;
  bool lookForMain{file.type == FileType::PairedSrc || file.type == FileType::UnpairedSrc};
  IncludeGuardCtx ctx{file.type == FileType::Hdr, includeGuardPat};
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
      else while(code[i] != '"') i++;
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
        Directive directive{code.substr(start, end - start)};
        switch(getDirectiveAction(directive, ctx)) {
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
          logIfVerbose("Found a main function...");
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