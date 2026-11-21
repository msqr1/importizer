// The unit test runner. Unit tests are located here.

#pragma once
#include <string>
#include <vector>

auto runUnitTest(const char *suiteName, const std::vector<std::string> &args) noexcept -> int;
