#include "Regex.hpp"
#include "Base.hpp"
#include <cstddef>
#include <cstdint>
#include <optional>
#include <source_location>
#include <string_view>
#define PCRE2_CODE_UNIT_WIDTH 8
#include "../3rdParty/pcre2/src/pcre2.h.generic"

namespace re {

namespace {

void ckPCRE2Code(int status, const std::source_location& loc
  = std::source_location::current()) {

  // Code inside this range is OK
  if(status > -2 && status < 101) return;
  char errMsg[256];
  pcre2_get_error_message(status, reinterpret_cast<PCRE2_UCHAR*>(errMsg), 256);
  exitWithErr(loc, "Regex: {}", errMsg);
}

} // namespace

Capture::Capture(): start{}, end{} {};
Capture::Capture(size_t start, size_t end): start{start}, end{end} {}
Captures::Captures() {}
Captures::Captures(size_t* ovector): ovector{ovector} {}
Capture Captures::operator[](int idx) const {
  
  // ovector comes in pairs of (start, end), so multiply by 2 to get correct index
  idx *= 2;
  return {ovector[idx], ovector[idx + 1]};
}
Pattern::Pattern() {}
Pattern::Pattern(Pattern&& other) noexcept: pattern{other.pattern},
  matchData{other.matchData} {
  other.pattern = nullptr;
  other.matchData = nullptr;
}
Pattern::Pattern(std::string_view pat, uint32_t opts) {
  reset(pat, opts);
}
Pattern& Pattern::reset(std::string_view pat, uint32_t opts) {
  pcre2_code_free(pattern);
  pcre2_match_data_free(matchData);
  int status;
  size_t _; // Unused
  pattern = pcre2_compile(reinterpret_cast<PCRE2_SPTR>(pat.data()), pat.length(), 
    opts | PCRE2_UTF | PCRE2_NO_UTF_CHECK, &status, &_, nullptr);
  ckPCRE2Code(status);
  status = pcre2_jit_compile(pattern, PCRE2_JIT_COMPLETE);
  if(status != PCRE2_ERROR_JIT_BADOPTION) ckPCRE2Code(status);
  matchData = pcre2_match_data_create_from_pattern(pattern, nullptr);
  if(matchData == nullptr) exitWithErr("Regex: Unable to allocate memory for match");
  return *this;
}
Pattern::~Pattern() {
  pcre2_code_free(pattern);
  pcre2_match_data_free(matchData);
}
std::optional<Captures> Pattern::match(std::string_view subject, size_t startOffset)
  const {
  int count{pcre2_match(pattern, reinterpret_cast<PCRE2_SPTR>(subject.data()),
    subject.length(), startOffset, PCRE2_NO_UTF_CHECK, matchData, nullptr)};
  ckPCRE2Code(count);
  if(count < 1) return std::nullopt;
  return pcre2_get_ovector_pointer(matchData);
}

} // namespace re