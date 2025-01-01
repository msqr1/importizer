#pragma once
#include <vector>
#include <string>

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

std::vector<Directive> lexDirectives(std::string& code);