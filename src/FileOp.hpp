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
  enum class Type {
    Hdr,
    Src,
    PairedSrc
  } type;
  bool manualExportingRequired;
  std::filesystem::path path;
  std::string content;
};

cppcoro::generator<File&> iterateFiles(const Opts& opts);
