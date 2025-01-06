#include "FileOp.hpp"
#include "Base.hpp"
#include "ArgProcessor.hpp"
#include "../3rdParty/Generator.hpp"
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

cppcoro::generator<File&> iterateFiles(const Opts& opts) {
  File file;
  fs::path path;
  fs::path relPath;
  std::string_view ext;
  std::ifstream ifs;
  std::ofstream ofs;
  logIfVerbose("Scanning input directory...");
  for(const auto& ent : fs::recursive_directory_iterator(opts.inDir)) {
    path = ent.path();
    relPath = path.lexically_relative(opts.inDir);
    logIfVerbose("Current file: {}", relPath.native());
    ext = relPath.extension().native();
    if(ext == opts.hdrExt) {
      for(const fs::path& p : opts.ignoredHeaders) {
        if(relPath == p) {
          logIfVerbose("Is ignored (in ignoredHeaders)");
          fs::copy_file(path, opts.outDir / relPath, 
            fs::copy_options::overwrite_existing);
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
    else {
      logIfVerbose("Is ignored, not header/source");
      continue;
    }
    logIfVerbose("Is a header/source, reading...");
    ifs.open(path);
    if(!ifs) exitWithErr("Unable to open {} for reading", relPath.native());

    // goto jumps over initialization of fsize
    {
      size_t fsize{fs::file_size(path)};
      file.content.resize_and_overwrite(fsize, [&](char* newBuf, size_t _) {
        ifs.read(newBuf, fsize);
        return fsize;
      });
    }
    if(ifs.fail() || ifs.bad()) exitWithErr("Unable to read from {}", relPath.native());
    ifs.close();
    file.relPath = std::move(relPath);
    co_yield file;
    relPath = std::move(file.relPath);
    logIfVerbose("Trying to write...");
    if(file.type == FileType::Hdr) relPath.replace_extension(opts.moduleInterfaceExt);
    relPath = opts.outDir / relPath;
    fs::create_directories(relPath.parent_path());
    ofs.open(relPath);
    if(!ofs) exitWithErr("Unable to open {} for writing", relPath.native());
    ofs.write(file.content.data(), file.content.length());
    if(ofs.fail() || ofs.bad()) exitWithErr("Unable to write to {}", relPath.native());
    ofs.close();
    skipThisFile:
  }
}