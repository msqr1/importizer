#pragma once
#include <string_view>
#include <optional>

enum class StdIncludeType : char {
  CppOrCwrap,
  CCompat,
};

// std::nullopt when not a system include
std::optional<StdIncludeType> getStdIncludeType(std::string_view include);