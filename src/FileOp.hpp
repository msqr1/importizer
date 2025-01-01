#pragma once
#include "Base.hpp"
#include <vector>
#include <filesystem>

namespace re {
  class Pattern;
}
namespace cppcoro {
  template<typename T> class generator;
}
struct Opts;
struct File {
  bool isHdr;
  bool paired;
  bool manualExport{};
  std::filesystem::path path; // Relative to inDir
  std::string content;
};

cppcoro::generator<File&> iterateFiles(const Opts& opts);
