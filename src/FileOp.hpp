#pragma once
#include "Base.hpp"
#include <vector>
#include <filesystem>
#include <variant>

namespace re {
  class Pattern;
}
namespace cppcoro {
  template<typename T> class generator;
}
struct Opts;

enum class FileType : char {
  Hdr,
  UnpairedSrc,
  SrcWithMain,
  PairedSrc
};
struct File {
  FileType type;
  std::filesystem::path relPath; // Relative to inDir/outDir
  std::string content;
};

cppcoro::generator<File&> iterateFiles(const Opts& opts);
