#include "FileOp.hpp"
#include "Regex.hpp"
#include "ArgProcessor.hpp"
#include "../3rdParty/Generator.hpp"
#include <fstream>

namespace fs = std::filesystem;

cppcoro::generator<File&> iterateFiles(const Opts& opts) {
  File file;
  fs::path path;
  std::string_view ext;
  std::ifstream ifs;
  std::ofstream ofs;
  logIfVerbose("Scanning input directory...");
  for(const auto& ent : fs::recursive_directory_iterator(opts.inDir)) {
    path = ent.path();
    logIfVerbose("Current file: {}", path.native());
    ext = path.extension().native();
    if(ext == opts.hdrExt) file.isHdr = true;
    else if(ext == opts.srcExt) {
      file.isHdr = false;
      path.replace_extension(opts.hdrExt);
      if(fs::exists(path)) file.paired = true;
      path.replace_extension(opts.srcExt);
    }
    else {
      logIfVerbose("Is ignored, not header/source");
      continue;
    }
    logIfVerbose("Is a header/source, reading...");
    ifs.open(opts.inDir / path);
    if(!ifs) exitWithErr("Unable to open {} for reading", path.native());
    size_t fsize{fs::file_size(path)};
    file.content.resize_and_overwrite(fsize, [&](char* newBuf, size_t _) {
      ifs.read(newBuf, fsize);
      return fsize;
    });
    if(ifs.fail() || ifs.bad()) exitWithErr("Unable to read from {}", path.native());
    ifs.close();
    path = file.path = path.lexically_relative(opts.inDir);
    co_yield file;
    logIfVerbose("Trying to write...");
    if(file.isHdr) path.replace_extension(opts.moduleInterfaceExt);
    path = opts.outDir / path;
    fs::create_directories(path.parent_path());
    ofs.open(path);
    if(!ofs) exitWithErr("Unable to open {} for writing", path.native());
    ofs.write(file.content.data(), file.content.length());
    if(ofs.fail() || ofs.bad()) exitWithErr("Unable to write to {}", path.native());
    ofs.close();
  }
}