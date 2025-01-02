#pragma once
#include <vector>
#include <string>
#include "FileOp.hpp"

enum class DirectiveType {
  Define,
  Undef,
  Condition,
  EndIf,
  Include,
  PragmaOnce,
  Other
};
struct Directive {
  DirectiveType type;
  std::string str;
  Directive(DirectiveType type_, std::string_view str_);
};
struct LexResult {
  std::vector<Directive> directives;
};

LexResult lex(std::string& code);