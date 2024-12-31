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
  std::string val;
  Directive(Type type_, std::string_view val_);
};

std::vector<Directive> lexDirectives(std::string& code);