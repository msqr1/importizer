#pragma once
#include <vector>
#include <string>
#include "FileOp.hpp"

struct Directive {
  enum class Type {
    Define,
    Undef,
    Condition,
    EndIf,
    Include,
    PragmaOnce,
    Other
  } type;
  std::string str;
  Directive(Type type_, std::string_view str_);
};
struct LexResult {
  std::vector<Directive> directives;
};

LexResult lex(std::string& code);