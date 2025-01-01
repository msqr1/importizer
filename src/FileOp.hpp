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
  bool pairedHdrIgnored;
  std::filesystem::path relPath; // Relative to inDir/outDir
  std::string content;
};

cppcoro::generator<File&> iterateFiles(const Opts& opts);
