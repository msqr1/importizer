#include "Base.hpp"
#include "ArgProcessor.hpp"
#include "Directive.hpp"
#include "FileOp.hpp"
#include "Preprocessor.hpp"
#include "Preamble.hpp"
#include <fmt/base.h>
#include <exception>
#include <vector>

void run(int argc, const char* const* argv) {
  const Opts opts{getOptsOrExit(argc, argv, verbose)};
  for(File& file : iterateFiles(opts)) {
    const std::vector<Directive> directives{
      preprocess(opts.transitionalOpts, file, opts.includeGuardPat)};
    bool manualExport{insertPreamble(file, directives, opts)};
    if(manualExport) {
      if(file.type == FileType::Hdr) {
        file.relPath.replace_extension(opts.moduleInterfaceExt);
      }
      log("{}", file.relPath.native());
    }
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
