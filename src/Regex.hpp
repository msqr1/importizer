#pragma once
#include <cstdint>
#include <string_view>
#include <optional>

struct pcre2_real_code_8;
struct pcre2_real_match_data_8;
namespace re {

class Capture {
friend class Captures;
  Capture(uintmax_t start, uintmax_t end);
public:
  uintmax_t start;
  uintmax_t end;
  Capture();
};

// Captures doesn't own anything, it's just a pointer to the ovector, so we can copy
class Captures {
friend class Pattern;
  uintmax_t* ovector;
public:
  Captures();
  Captures(uintmax_t* ovector);
  Capture operator[](int idx) const;
};

// Pattern MANAGES resources, so we will prohibit copying (like a unique_ptr)
class Pattern {
  pcre2_real_code_8* pattern{};
  pcre2_real_match_data_8* matchData{};
public:
  Pattern(const Pattern&) = delete;
  Pattern& operator=(const Pattern&) = delete;
  Pattern(Pattern&& other) noexcept;
  Pattern(std::string_view pat, uint32_t opts = 0);
  Pattern& reset(std::string_view pat, uint32_t opts = 0);
  Pattern();
  ~Pattern();
  std::optional<Captures> match(std::string_view subject, uintmax_t startOffset = 0) const;
};

} // namespace re

