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
  size_t startOffset;
  std::string_view includeStr;
  IncludeInfo(size_t startOffset, bool isAngle, std::string_view includeStr);
};

// Only for ifndef and define directive
struct GuardInfo {
  size_t startOffset;
  std::string_view identifier;
  GuardInfo(size_t startOffset, std::string_view identifier);
};

struct Directive {
  DirectiveType type;
  std::string str;

  // Only valid for the include, ifndef and define directive
  std::variant<std::monostate, IncludeInfo, GuardInfo> info;
  Directive(std::string&& str);
  Directive(Directive&& other);
};
struct PreprocessResult {
  std::vector<Directive> directives;
};

PreprocessResult preprocess(File& file, const std::optional<re::Pattern>& maybeIncludeGuardPat);