#include "FileOp.hpp"
#include "Base.hpp"
#include "OptProcessor.hpp"
#include <fmt/std.h>
#include <cstddef>
#include <fstream>
#include <filesystem>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace fs = std::filesystem;
void readFromPath(const fs::path& path, std::string& str) {
  std::ifstream ifs{path, std::ios::binary};
  if(!ifs) exitWithErr("Unable to open {} for reading", path);
  size_t fsize{fs::file_size(path)};
  str.resize_and_overwrite(fsize, [&](char* newBuf, [[maybe_unused]] size_t _) {
    ifs.read(newBuf, fsize);
    return fsize;
  });
#ifdef WIN32
  std::erase(str, '\r');
#endif
  if(!ifs) exitWithErr("Unable to read from {}", path);
}
void writeToPath(const fs::path& path, std::string_view data) {
  fs::create_directories(path.parent_path());
  std::ofstream ofs{path};
  if(!ofs) exitWithErr("Unable to open {} for writing", path);
  ofs.write(data.data(), data.length());
  if(!ofs) exitWithErr("Unable to write to {}", path);
}
std::vector<File> getProcessableFiles(const Opts& opts) {
  std::vector<File> files;
  fs::path path;
  fs::path relPath;
  fs::path ext;
  for(const auto& ent : fs::recursive_directory_iterator(opts.inDir)) {
    if(!ent.is_regular_file()) continue;
    path = ent.path();
    relPath = path.lexically_relative(opts.inDir);
    ext = relPath.extension();
    File file;
    if(ext == opts.hdrExt) {
      for(const fs::path& p : opts.ignoredHeaders) {
        if(relPath == p) {
          relPath = opts.outDir / relPath;
          fs::create_directories(relPath.parent_path());
          fs::copy_file(path, relPath, fs::copy_options::overwrite_existing);
          goto skipThisFile;
        }
      }
      file.type = FileType::Hdr;
    }
    else if(ext == opts.srcExt) {
      file.type = FileType::UnpairedSrc;
      path.replace_extension(opts.hdrExt);
      if(fs::exists(path)) file.type = FileType::PairedSrc;
      path.replace_extension(opts.srcExt);
      relPath.replace_extension(opts.hdrExt);
      for(const fs::path& p : opts.ignoredHeaders) {
        if(relPath == p) {
          file.type = FileType::SrcWithMain;
          break;
        }
      }
      relPath.replace_extension(opts.srcExt);
    }
    else continue;
    file.path = std::move(path);
    file.relPath = std::move(relPath);
    files.emplace_back(std::move(file));
    skipThisFile:;
  }
  return files;
}