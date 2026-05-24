#pragma once

#include <cstdio>
#include <filesystem>
#include <memory>
#include <string>

namespace fs = std::filesystem;

struct FileCloser {
  void operator()(std::FILE *f) const { std::fclose(f); }
};
using File = std::unique_ptr<std::FILE, FileCloser>;

// For handling wchar_t paths on Windows
File portableFOpen(const fs::path &path);

// 1st arg is raw pointer, but caller is expected to pass File::get()
void readToStr(std::FILE *f, std::string &s, const fs::path &path);
void readToStr(const fs::path &path, std::string &s);
