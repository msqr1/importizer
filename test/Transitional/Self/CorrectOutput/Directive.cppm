#pragma once
#include "Export.hpp"
#ifdef CPP_MODULES
module;
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
export module Directive;
import FileOp;
#else
#include "FileOp.cppm"
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#endif


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
class IncludeGuardCtx {
public:
  IncludeGuardState state;
  size_t counter;
  const re::Pattern& pat;
  IncludeGuardCtx(FileType type, const re::Pattern& pat):
    state{type == FileType::Hdr ?
      IncludeGuardState::Looking : IncludeGuardState::NotLooking},
    pat{pat} {}
};
class IncludeInfo {
public:
  bool isAngle;
  size_t startOffset;
  std::string_view includeStr;
  IncludeInfo(bool isAngle, size_t startOffset, std::string_view includeStr);
};

// For ifndef and define that matches opts.includeGuardPat
struct IncludeGuard {};

// For else statement, only possible if type is ElCond to differentiate from el(...)
struct Else {};
enum class DirectiveType : char {
  Define,
  Undef,
  IfCond,
  Else,
  ElCond,
  EndIf,
  Include,
  PragmaOnce,
  Other
};
class Directive {
public:
  DirectiveType type;
  std::string str;

  // Only hold other information beside the type
  std::variant<std::monostate, IncludeInfo, IncludeGuard> extraInfo;
  Directive(std::string&& str, const IncludeGuardCtx& ctx);
  Directive(Directive&& other) noexcept;
  Directive(const Directive& other);
};
enum class StdIncludeType : char {
  CppOrCwrap,
  CCompat,
};

// std::nullopt when not a system include
std::optional<StdIncludeType> getStdIncludeType(std::string_view include);
