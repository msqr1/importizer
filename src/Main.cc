#include "Base.hpp"
#include "ArgProcessor.hpp"
#include "FileOp.hpp"
#include "Directive.hpp"
#include "Modularizer.hpp"
#include "../3rdParty/fmt/include/fmt/format.h"
#include "../3rdParty/Generator.hpp"
#include <exception>

void run(int argc, const char* const* argv) {
  const Opts opts{getOptsOrExit(argc, argv, verbose)};
  for(File& file : iterateFiles(opts)) {
    bool manualExport{modularize(file, lexDirectives(file.content), opts)};
    if(manualExport) {
      if(file.isHdr) file.relPath.replace_extension(opts.moduleInterfaceExt);
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
