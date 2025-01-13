#include "ArgProcessor.hpp"
#include "Directive.hpp"
#include "FileOp.hpp"
#include "Preamble.hpp"
#include "Preprocessor.hpp"
#include <fmt/base.h>
#include <array>
#include <cmath>
#include <exception>
#include <string>
#include <string_view>
#include <vector>

void run(int argc, const char* const* argv) {
  const Opts opts{getOptsOrExit(argc, argv)};
  if(opts.transitionalOpts) {
    const TransitionalOpts& t{*opts.transitionalOpts};
    std::string exportMacros{"#ifdef "};
    const std::array<std::string_view, 14> tokens{t.mi_control, "\n#define ", 
      t.mi_exportKeyword, " export\n#define ", t.mi_exportBlockBegin,
      " export {\n#define ", t.mi_exportBlockEnd, " }\n#else\n#define ",
      t.mi_exportKeyword, "\n#define ", t.mi_exportBlockBegin, "\n#define ",
      t.mi_exportBlockEnd, "\n#endif"};
    for(std::string_view token : tokens) exportMacros += token;
    writeToPath(opts.outDir / t.exportMacrosPath, exportMacros);
  }
  std::vector<File> processableFiles{getProcessableFiles(opts)};
  for(File& file : processableFiles) {
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
  }
  catch(int exitCode) {
    return exitCode;
  }
}
