#pragma once
#include <vector>
#include <string>
#include <variant>

struct File;
struct Opts;
enum class DirectiveType : char {
  Define,
  Undef,
  Ifndef,
  IfCond,
  ElCond,
  EndIf,
  Include,
  PragmaOnce,
  Other
};
struct IncludeInfo {
  bool isAngle;
  std::string_view includeStr;
  IncludeInfo(bool isAngle, std::string_view includeStr);
};

struct Directive {
  DirectiveType type;
  std::string str;

  // Only valid for the include, ifndef and define directive
  std::variant<std::monostate, IncludeInfo, std::string_view> info;
  Directive(std::string&& str);
  Directive(Directive&& other);
};
struct PreprocessResult {
  std::vector<Directive> directives;
};

PreprocessResult preprocess(File& file, const std::optional<re::Pattern>& maybeIncludeGuardPat);