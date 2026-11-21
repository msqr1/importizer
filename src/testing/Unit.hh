// The unit test runner. Unit tests are located here.

#pragma once
#include <string>
#include <vector>

auto runUnitTest(char const *suiteName, std::vector<std::string> const &args) noexcept -> int;
