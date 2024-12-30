#pragma once
#include "Regex.hpp"
#include <filesystem>
#include <string>
#include <vector>

struct Opts {
  bool raiseDefine;
  std::filesystem::path inDir;
  std::filesystem::path outDir;
  std::optional<re::Pattern> maybeIncludeGuardPat;
  std::string hdrExt;
  std::string srcExt;
  std::string moduleInterfaceExt;
  std::vector<std::filesystem::path> includePaths;
};

Opts getOptsOrExit(int argc, const char* const* argv, bool& verbose);
