#include "Base.hpp"
#include "ArgProcessor.hpp"
#include "FileOp.hpp"
#include "Preprocessor.hpp"
#include "Modularizer.hpp"
#include "fmt/base.h"
#include <exception>

void run(int argc, const char* const* argv) {
  const Opts opts{getOptsOrExit(argc, argv, verbose)};
  for(File& file : iterateFiles(opts)) {
    const PreprocessResult pr{preprocess(file, opts.includeGuardPat)};
    const bool manualExport{modularize(file, pr, opts)};
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
