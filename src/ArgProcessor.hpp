#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include <optional>

namespace re {
  class Pattern;
}

struct Opts {
  bool raiseDefine;
  std::filesystem::path inDir;
  std::filesystem::path outDir;
  std::optional<re::Pattern> maybeIncludeGuardPat;
  std::string hdrExt;
  std::string srcExt;
  std::string moduleInterfaceExt;
  std::vector<std::filesystem::path> includePaths;
  std::vector<std::filesystem::path> ignoredHeaders;
};

Opts getOptsOrExit(int argc, const char* const* argv, bool& verbose);
