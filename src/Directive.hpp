#pragma once
#include <optional>
#include <variant>

namespace re {
  class Pattern;
}

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

enum class IncludeGuardState : char {
  NotLooking,
  Looking,
  GotIfndef,
  GotDefine,
  GotEndif
};

struct IncludeGuardCtx {
  IncludeGuardState state;
  const std::optional<re::Pattern>& pat;
  size_t counter;
  IncludeGuardCtx(bool lookFor, const std::optional<re::Pattern>& pat);
};

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

struct Directive {
  DirectiveType type;
  std::string str;

  // Only valid for the include, ifndef and define directive
  std::variant<std::monostate, IncludeInfo, GuardInfo> info;
  Directive(std::string&& str);
  Directive(Directive&& other);
};

enum class DirectiveAction : char {
  Ignore,
  EmplaceRemove,
  Remove
};

DirectiveAction getDirectiveAction(const Directive& directive, IncludeGuardCtx& ctx);