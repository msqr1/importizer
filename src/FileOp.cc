#include "FileOp.hpp"
#include "Util.hpp"
#include "OptProcessor.hpp"
#include <cstddef>
#include <fmt/std.h>
#include <fstream>
#include <filesystem>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <version>

namespace fs = std::filesystem;
void readFromPath(const fs::path& path, std::string& str) {
  std::ifstream ifs{path, std::fstream::binary};
  if(!ifs) exitWithErr("Unable to open {} for reading", path);
  
  // file_size return an uintmax_t which may not be size_t
  size_t fsize{static_cast<size_t>(fs::file_size(path))};
#ifdef __cpp_lib_string_resize_and_overwrite
  str.resize_and_overwrite(fsize, [fsize]
    ([[maybe_unused]] char* _, [[maybe_unused]] size_t _1) {
    return fsize;
  });
#else
  str.resize(fsize);
#endif
  ifs.read(str.data(), fsize);
  if(!ifs) exitWithErr("Unable to read from {}", path);
#ifdef WIN32
  
  // Normalize line endings
  std::erase(str, '\r');
#endif
}
void writeToPath(const fs::path& path, std::string_view data) {
  fs::create_directories(path.parent_path());
  std::ofstream ofs{path};
  if(!ofs) exitWithErr("Unable to open {} for writing", path);
  ofs.write(data.data(), data.length());
  if(!ofs) exitWithErr("Unable to write to {}", path);
}
File::File(FileType type, std::filesystem::path& path, std::filesystem::path& relPath)
  noexcept: type{type}, path{std::move(path)}, relPath{std::move(relPath)} {}
std::vector<File> getHeadersAndSources(const Opts& opts) {
  std::vector<File> files;
  for(const auto& ent : fs::recursive_directory_iterator(opts.inDir)) {
    if(!ent.is_regular_file()) continue;
    fs::path path{ent.path()};
    fs::path relPath{path.lexically_relative(opts.inDir)};
    fs::path ext{relPath.extension()};
    FileType type;
    if(ext == opts.hdrExt) {
      for(const fs::path& p : opts.ignoredHdrs) {
        if(relPath == p) {
          relPath = opts.outDir / relPath;
          fs::create_directories(relPath.parent_path());
          fs::copy_file(path, relPath, fs::copy_options::overwrite_existing);
          goto skipThisFile;
        }
      }
      type = FileType::Hdr;
      for(const fs::path& p : opts.umbrellaHdrs) {
        if(relPath == p) {
          type = FileType::UmbrellaHdr;
          break;
        }
      }
    }
    else if(ext == opts.srcExt) {
      type = FileType::UnpairedSrc;
      path.replace_extension(opts.hdrExt);
      if(fs::exists(path)) type = FileType::PairedSrc;
      path.replace_extension(opts.srcExt);
      relPath.replace_extension(opts.hdrExt);
      for(const fs::path& p : opts.ignoredHdrs) {
        if(relPath == p) {
          type = FileType::SrcWithMain;
          break;
        }
      }
      relPath.replace_extension(opts.srcExt);
    }
    else continue;
    files.emplace_back(type, path, relPath);
    skipThisFile:;
  }
  return files;
}