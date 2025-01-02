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
struct HdrInfo {};
;

struct File {
  enum class Type {
    Hdr,
    UnpairedSrc,
    SrcWithMain,
    PairedSrc
  } type;
  std::filesystem::path relPath; // Relative to inDir/outDir
  std::string content;
};

cppcoro::generator<File&> iterateFiles(const Opts& opts);
