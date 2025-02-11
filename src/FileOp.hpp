#pragma once
#include <string>
#include <filesystem>
#include <string_view>
#include <vector>

struct Opts;
enum class FileType : char {
  Hdr,
  UmbrellaHdr,
  //IgnoredHdr, // Only possible in transitional mode
  UnpairedSrc,
  SrcWithMain,
  PairedSrc
};
struct File {
  FileType type;
  std::filesystem::path path; // Full path
  std::filesystem::path relPath; // Relative to inDir/outDir
  std::string content;
};
void readFromPath(const std::filesystem::path& path, std::string& str);
void writeToPath(const std::filesystem::path& to, std::string_view content);
std::vector<File> getProcessableFiles(const Opts& opts);
