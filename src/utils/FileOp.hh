#pragma once
#include <cstdio>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>

namespace fs = std::filesystem;

struct FileCloser {
  void operator()(std::FILE *f) const { std::fclose(f); }
};

using File = std::unique_ptr<std::FILE, FileCloser>;

// For handling wchar_t paths on Windows
[[nodiscard]] File openFile(const fs::path &path,
                            std::string_view mode = "r") noexcept;

// 1st arg is raw pointer, but caller is expected to pass File::get()
[[nodiscard]] bool readToStr(std::FILE *f, std::string &s,
                             const fs::path &path) noexcept;

[[nodiscard]] bool readToStr(const fs::path &path, std::string &s) noexcept;
