#pragma once
#include "FileOp.hpp"
#include <cstddef>
#include <string>
#include <string_view>
#include <variant>

namespace re {
  class Pattern;
}

enum class IncludeGuardState : char {
  NotLooking,
  Looking,
  GotIfndef,
  GotDefine,
  GotEndif
};

struct IncludeGuardCtx {
  IncludeGuardState state;
  const re::Pattern& pat;
  size_t counter;
  IncludeGuardCtx(FileType type, const re::Pattern& pat):
    state{type == FileType::Hdr ? 
      IncludeGuardState::Looking : IncludeGuardState::NotLooking},
    pat{pat} {}
};

struct IncludeInfo {
  bool isAngle;
  size_t startOffset;
  std::string_view includeStr;
  IncludeInfo(size_t startOffset, bool isAngle, std::string_view includeStr);
};

// For ifndef and define that matches opts.includeGuardPat
struct IncludeGuard {};

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

  std::variant<std::monostate, IncludeInfo, IncludeGuard> extraInfo;
  Directive(std::string&& str, const IncludeGuardCtx& ctx);
  Directive(Directive&& other);
};