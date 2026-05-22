#pragma once
#include <filesystem>
#include <string>
#include <string_view>

void readFrom(const std::filesystem::path &path, std::string &str);
void writeTo(const std::filesystem::path &to, std::string_view content);
