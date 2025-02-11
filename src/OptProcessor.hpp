#pragma once
#include "Regex.hpp"
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

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
  std::filesystem::path hdrExt;
  std::filesystem::path srcExt;
  std::filesystem::path moduleInterfaceExt;
  std::vector<std::filesystem::path> includePaths;
  std::vector<std::filesystem::path> ignoredHdrs;
  std::vector<std::filesystem::path> umbrellaHdrs;
  std::optional<std::string> bootstrapStdModule;
  std::optional<TransitionalOpts> transitionalOpts;
};
Opts getOptsOrExit(int argc, const char* const* argv);
