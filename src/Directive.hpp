#pragma once
#include "FileOp.hpp"
#include <cstddef>
#include <optional>
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
  size_t counter;
  const re::Pattern& pat;
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
  Directive(Directive&& other) noexcept;
};
enum class StdIncludeType : char {
  CppOrCwrap,
  CCompat,
};

// std::nullopt when not a system include
std::optional<StdIncludeType> getStdIncludeType(std::string_view include);