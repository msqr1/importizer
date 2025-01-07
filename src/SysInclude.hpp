#pragma once
#include <string_view>
#include <optional>

enum class SysIncludeType : char {
  CppOrCwrap,
  CCompat,
};

// std::nullopt when not a system include
std::optional<SysIncludeType> getSysIncludeType(std::string_view include);