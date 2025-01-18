#include "OptProcessor.hpp"
#include "Directive.hpp"
#include "FileOp.hpp"
#include "Preamble.hpp"
#include "Preprocessor.hpp"
#include <fmt/base.h>
#include <fmt/format.h>
#include <cmath>
#include <exception>
#include <vector>

void run(int argc, const char* const* argv) {
  const Opts opts{getOptsOrExit(argc, argv)};
  if(opts.transitionalOpts) {
    const TransitionalOpts& t{*opts.transitionalOpts};
    writeToPath(opts.outDir / t.exportMacrosPath, fmt::format(
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
  for(File& file : getProcessableFiles(opts)) {
    if(opts.logCurrentFile) log("Current file: {}", file.relPath.native());
    readFromPath(file.path, file.content);
    const std::vector<Directive> directives{
      preprocess(opts.transitionalOpts, file, opts.includeGuardPat)};
    bool manualExport{insertPreamble(file, directives, opts)};
    if(file.type == FileType::Hdr) {
      file.relPath.replace_extension(opts.moduleInterfaceExt);
    }
    if(manualExport) log("{}", file.relPath.native());
    writeToPath(opts.outDir / file.relPath, file.content);
  }
}
int main(int argc, const char* const* argv) {
  try {
    run(argc, argv);
  }
  catch(const std::exception& exc) {
    fmt::println(stderr, "std::exception thrown: {}", exc.what());
    return 1;
  }
  catch(int exitCode) {
    return exitCode;
  }
}
