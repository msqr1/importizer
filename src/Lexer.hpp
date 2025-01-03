#pragma once
#include <vector>
#include <string>

struct File;
enum class DirectiveType : char {
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

LexResult lex(File& file);