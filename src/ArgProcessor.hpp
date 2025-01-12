#pragma once
#include "Regex.hpp"
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

struct TransitionalOpts {
  std::string mi_control;
  std::string mi_exportKeyword;
  std::string mi_exportBlockBegin;
  std::string mi_exportBlockEnd;
  std::filesystem::path exportMacrosPath;
};
struct Opts {
  bool stdInclude2Import;
  std::filesystem::path inDir;
  std::filesystem::path outDir;
  re::Pattern includeGuardPat;
  std::string hdrExt;
  std::string srcExt;
  std::string moduleInterfaceExt;
  std::vector<std::filesystem::path> includePaths;
  std::vector<std::filesystem::path> ignoredHeaders;
  std::optional<TransitionalOpts> transitionalOpts;
};

Opts getOptsOrExit(int argc, const char* const* argv, bool& verbose);
