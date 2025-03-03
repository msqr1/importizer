#include "Directive.hpp"
#include "Base.hpp"
#include "FileOp.hpp"
#include "OptProcessor.hpp"
#include "Regex.hpp"
#include <array>
#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

namespace fs = std::filesystem;
IncludeGuardCtx::IncludeGuardCtx(FileType type, const std::optional<Regex>& pat): 
  state{type == FileType::Hdr && pat ?
  IncludeGuardState::Looking : IncludeGuardState::NotLooking} {}
IncludeInfo::IncludeInfo(bool isAngle, size_t startOffset, std::string_view includeStr):
  isAngle{isAngle}, startOffset{startOffset}, includeStr{includeStr} {}
Directive::Directive(std::string&& str_, const IncludeGuardCtx& ctx, const Opts& opts):
  str{std::move(str_)} {
  if(str.back() != '\n') str += '\n';
  auto getWord = [](size_t start, std::string_view str) {
    while(str[start] == ' ') ++start;
    size_t end{start};
    while(str[end] != ' ' && str[end] != '\n'
      && str[end] != '/') ++end;
    return str.substr(start, end - start);
  };
  std::string_view directive{getWord(1, str)};
  std::string_view first2Chars{directive.substr(0, 2)};
  if(directive == "define") type = DirectiveType::Define;
  else if(directive == "undef") type = DirectiveType::Undef;
  else if(directive == "include") {
    type = DirectiveType::Include;
    size_t start{str.find('<', 1 + directive.length())};
    size_t end;
    bool isAngle{start != notFound};
    if(isAngle) {
      start++;
      end = str.find('>', start);
    }
    else {
      start = str.find('"') + 1;
      end = str.find('"', start);
    }
    extraInfo.emplace<IncludeInfo>(isAngle, start,
      std::string_view(str.c_str() + start, end - start));
  }
  else if(directive == "endif") type = DirectiveType::EndIf; 
  else if(first2Chars == "if") type = DirectiveType::IfCond;
  else if(directive == "else") type = DirectiveType::Else;
  else if(first2Chars == "el") type = DirectiveType::ElCond;
  else if(opts.pragmaOnce && directive == "pragma" &&
    getWord(1 + directive.length(), str) == "once") {
    type = DirectiveType::PragmaOnce;
  }
  else type = DirectiveType::Other;
  if(((ctx.state == IncludeGuardState::GotIfndef && type == DirectiveType::Define) ||
    (ctx.state == IncludeGuardState::Looking && directive == "ifndef")) &&
    opts.includeGuard->match(getWord(1 + directive.length(), str))) {
    extraInfo.emplace<IncludeGuard>();
  }
}
Directive::Directive(Directive&& other) noexcept {
  operator=(std::move(other));
}
Directive& Directive::operator=(Directive&& other) noexcept {
  type = other.type;
  extraInfo = std::move(other.extraInfo);
  str = std::move(other.str);
  if(std::holds_alternative<IncludeInfo>(extraInfo)) {
    IncludeInfo& info{std::get<IncludeInfo>(extraInfo)};
    info.includeStr = {str.c_str() + info.startOffset, info.includeStr.length()};
  }
  return *this;
}
Directive::Directive(const Directive& other): type{other.type}, str{other.str},
  extraInfo(other.extraInfo) {
  if(std::holds_alternative<IncludeInfo>(extraInfo)) {
    IncludeInfo& includeInfo{std::get<IncludeInfo>(extraInfo)};
    size_t len{includeInfo.includeStr.length()};
    includeInfo.includeStr = std::string_view(str.c_str() + includeInfo.startOffset, len);
  }
}
namespace {

constexpr std::array<std::string_view, 114> cppOrCwrapHdrs {
  "cstdlib", "execution", "cfloat", "climits", "compare", "coroutine", "csetjmp",
  "csignal", "cstdarg", "cstddef", "cstdint", "exception", "initializer_list", "limits",
  "new", "source_location", "stdfloat", "typeindex", "typeinfo", "version", "concepts",
  "cassert", "cerrno", "debugging", "stacktrace", "stdexcept", "system_error", "memory",
  "memory_resource", "scoped_allocator", "ratio", "type_traits", "any", "bit", "bitset",
  "expected", "functional", "optional", "tuple", "utility", "variant", "array", "deque",
  "flat_map", "flat_set", "forward_list", "inplace_vector", "list", "map", "mdspan",
  "queue", "set", "span", "stack", "unordered_map", "unordered_set", "vector", "iterator",
  "generator", "ranges", "algorithm", "numeric", "cstring", "string", "string_view",
  "cctype", "charconv", "clocale", "codecvt", "cuchar", "cwchar", "cwctype", "format",
  "locale", "regex", "text_encoding", "cfenv", "cmath", "complex", "linalg", "numbers",
  "random", "simd", "valarray", "chrono", "ctime", "cinttypes", "cstdio", "filesystem",
  "fstream", "iomanip", "ios", "iosfwd", "iostream", "istream", "ostream", "print",
  "spanstream", "sstream", "streambuf", "strstream", "syncstream", "atomic", "barrier",
  "condition_variable", "future", "hazard_pointer", "latch", "mutex", "rcu", "semaphore",
  "shared_mutex", "stop_token", "thread"
};
constexpr std::array<std::string_view, 24> cCompatHdrs {
  "assert.h", "ctype.h", "errno.h", "fenv.h", "float.h", "inttypes.h", "limits.h", 
  "locale.h", "math.h", "setjmp.h", "signal.h", "stdarg.h", "stddef.h", "stdint.h",
  "stdio.h", "stdlib.h", "string.h", "time.h", "uchar.h", "wchar.h", "wctype.h",
  "stdatomic.h", "complex.h", "tgmath.h"
};

} // namespace
std::optional<StdIncludeType> getStdIncludeType(std::string_view include) {
  for(std::string_view cppOrCwrapHdr : cppOrCwrapHdrs) {
    if(include == cppOrCwrapHdr) return StdIncludeType::CppOrCwrap;
  }
  for(std::string_view cCompatHdr : cCompatHdrs) {
    if(include == cCompatHdr) return StdIncludeType::CCompat;
  }
  return std::nullopt;
}
std::optional<std::filesystem::path> resolveInclude(const ResolveIncludeCtx& ctx,
  const IncludeInfo& info, const std::filesystem::path& currentFilePath) {
  if(!info.isAngle) {
    fs::path p{currentFilePath};
    p.remove_filename();
    p /= info.includeStr;
    if(fs::exists(p)) {
      p = fs::relative(p, ctx.inDir);
      if(*p.begin() != "..") return p;
    }
  }
  for(const fs::path& includePath : ctx.includePaths) {
    fs::path p{includePath / info.includeStr};
    if(fs::exists(p)) {
      p = fs::relative(p, ctx.inDir);
      if(*p.begin() != "..") return p;
    }
  }
  return std::nullopt;
}