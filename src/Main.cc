#include "Base.hpp"
#include "OptProcessor.hpp"
#include "FileOp.hpp"
#include "Preamble.hpp"
#include "Preprocessor.hpp"
#include <exception>
#include <filesystem>

namespace fs = std::filesystem;
namespace {

void run(int argc, const char** argv) {
  const Opts opts{getOptsOrExit(argc, argv)};
  if(opts.transitional) {
    const TransitionalOpts& t{*opts.transitional};
    writeToPath(opts.outDir / t.exportMacrosPath, format(
      "#ifdef {0}\n"
      "#define {1} export\n"
      "#define {2} export {{\n"
      "#define {3} }}\n"
      "#else\n"
      "#define {1}\n"
      "#define {2}\n"
      "#define {3}\n"
      "#endif",
      t.mi_control, t.mi_exportKeyword, t.mi_exportBlockBegin,
      t.mi_exportBlockEnd));
  }
  fs::path backCompatHdr;
  for(File& file : getProcessableFiles(opts)) {
    if(opts.logCurrentFile) println("Current file: {}", file.relPath);
    readFromPath(file.path, file.content);
    bool manualExport{addPreamble(file, preprocess(opts, file), opts)};
    if(file.type == FileType::Hdr || file.type == FileType::UmbrellaHdr) {
      if(opts.transitional && opts.transitional->backCompatHdrs) {
        backCompatHdr = opts.outDir / file.relPath;
        writeToPath(backCompatHdr, format(
          "#include \"{}\"",
          file.relPath.replace_extension(opts.moduleInterfaceExt).filename()));
      }
      else file.relPath.replace_extension(opts.moduleInterfaceExt);
    }
    if(manualExport) println("{}", file.relPath);
    writeToPath(opts.outDir / file.relPath, file.content);
  }
}

}
int main(int argc, const char** argv) {
  try {
    run(argc, argv);
  }
  catch(const std::exception& exc) {
    println("std::exception thrown: {}", exc.what());
    return 1;
  }
  catch(int exitCode) {
    return exitCode;
  }
}