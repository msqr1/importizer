#pragma once
#include "Export.hpp"
#ifdef CPP_MODULES
module;
#include <filesystem>
#include <optional>
#include <string>
#include <vector>
export module OptProcessor;
import Regex;
#else
#include "Regex.cppm"
#include <filesystem>
#include <optional>
#include <string>
#include <vector>
#endif


struct TransitionalOpts {
  bool backCompatHdrs;
  std::string mi_control;
  std::string mi_exportKeyword;
  std::string mi_exportBlockBegin;
  std::string mi_exportBlockEnd;
  std::filesystem::path exportMacrosPath;
};
struct Opts {
  bool stdIncludeToImport;
  bool logCurrentFile;
  re::Pattern includeGuardPat;
  std::filesystem::path inDir;
  std::filesystem::path outDir;
  std::string hdrExt;
  std::string srcExt;
  std::string moduleInterfaceExt;
  std::vector<std::filesystem::path> includePaths;
  std::vector<std::filesystem::path> ignoredHeaders;
  std::optional<TransitionalOpts> transitionalOpts;
};
Opts getOptsOrExit(int argc, const char* const* argv);
