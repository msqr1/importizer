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
friend class Regex;
  uintmax_t* ovector;
public:
  Captures();
  Captures(uintmax_t* ovector);
  Capture operator[](int idx) const;
};

// Regex MANAGES resources, so we will prohibit copying (like a unique_ptr)
class Regex {
  pcre2_real_code_8* pattern{};
  pcre2_real_match_data_8* matchData{};
public:
  Regex(const Regex&) = delete;
  Regex& operator=(const Regex&) = delete;
  Regex(Regex&& other) noexcept;
  Regex(std::string_view pat, uint32_t opts = 0);
  Regex& reset(std::string_view pat, uint32_t opts = 0);
  Regex();
  ~Regex();
  std::optional<Captures> match(std::string_view subject, uintmax_t startOffset = 0) const;
};

} // namespace re

