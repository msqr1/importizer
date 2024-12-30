#include "FileOp.hpp"
#include "Regex.hpp"
#include "ArgProcessor.hpp"
#include "../3rdParty/Generator.hpp"
#include <fstream>

namespace fs = std::filesystem;

cppcoro::generator<File&> iterateFiles(const Opts& opts) {
  const size_t prefixLen = opts.inDir.native().length() + 1;
  File file;
  fs::path path;
  std::string_view pathView;
  std::ifstream ifs;
  std::ofstream ofs;
  logIfVerbose("Scanning input directory...");
  for(const auto& ent : fs::recursive_directory_iterator(opts.inDir)) {
    path = ent.path();
    logIfVerbose("Current file: {}", path.native());
    pathView = path.extension().native();
    if(pathView == opts.hdrExt) file.type = File::Type::Hdr
    else if(pathView == opts.srcExt) {
      path.replace_extension(opts.hdrExt);
      file.type = fs::exists(path) ? File::Type::PairedSrc : File::Type::Src;
      path.replace_extension(opts.srcExt);
    }
    else {
      logIfVerbose("Is ignored, not header/source");
      continue;
    }
    logIfVerbose("Is a header/source, reading...");
    ifs.open(path);
    if(!ifs) exitWithErr("Unable to open {} for reading", path.native());
    size_t fsize{fs::file_size(path)};
    file.content.resize_and_overwrite(fsize, [&](char* newBuf, size_t _) {
      ifs.read(newBuf, fsize);
      return fsize;
    });
    if(ifs.fail() || ifs.bad()) exitWithErr("Unable to read from {}", path.native());
    ifs.close();
    file.path = path;
    co_yield file;
    logIfVerbose("Trying to write...");
    if(file.type == File::Type::Hdr) path.replace_extension(opts.moduleInterfaceExt);
    pathView = path.native();
    pathView.remove_prefix(prefixLen);
    path = opts.outDir / pathView;
    fs::create_directories(path.parent_path());
    ofs.open(path);
    if(!ofs) exitWithErr("Unable to open {} for writing", path.native());
    ofs.write(file.content.data(), file.content.length());
    if(ofs.fail() || ofs.bad()) exitWithErr("Unable to write to {}", path.native());
    ofs.close();
  }
}