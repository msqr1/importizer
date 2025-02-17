#pragma once
#include "FileOp.hpp"
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace re {
  class Regex;
}
enum class IncludeGuardState : char {
  NotLooking,
  Looking,
  GotIfndef,
  GotDefine,
  GotEndIf
};
class IncludeGuardCtx {
public:
  IncludeGuardState state;
  uintmax_t counter;
  IncludeGuardCtx(FileType type, const std::optional<re::Regex>& pat);
};
class IncludeInfo {
public:
  bool isAngle;
  uintmax_t startOffset;
  std::string_view includeStr;
  IncludeInfo(bool isAngle, uintmax_t startOffset, std::string_view includeStr);
};

// For ifndef and define that matches opts.includeGuard
struct IncludeGuard {};
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
  Directive(std::string&& str, const IncludeGuardCtx& ctx, const Opts& opts);
  Directive(Directive&& other) noexcept;
  Directive& operator=(Directive&& other) noexcept;
  Directive(const Directive& other);
  Directive();
};
enum class StdIncludeType : char {
  CppOrCwrap,
  CCompat,
};

// std::nullopt when not a standard include
std::optional<StdIncludeType> getStdIncludeType(std::string_view include);

// Just for the sake of making the lines not irritatingly long
// Also these fixed from the start
struct ResolveIncludeCtx {
  const std::filesystem::path& inDir;
  const std::vector<std::filesystem::path>& includePaths;
};

// Resolve an include, the path of the include relative to inDir,
// return std::nullopt when the include doesn't exist, or not under inDir
std::optional<std::filesystem::path> resolveInclude(const ResolveIncludeCtx& ctx,
  const IncludeInfo& info, const std::filesystem::path& currentFilePath);