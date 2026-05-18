#pragma once
#include <filesystem>
#include <string>
#include <string_view>

void readFromPath(const std::filesystem::path &path, std::string &str);
void writeToPath(const std::filesystem::path &to, std::string_view content);
