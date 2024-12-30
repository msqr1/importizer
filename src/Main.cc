#include "Base.hpp"
#include "Regex.hpp"
#include "ArgProcessor.hpp"
#include "FileOp.hpp"
#include "../3rdParty/Generator.hpp"
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

void run(int argc, const char* const* argv) {
  const Opts opts{getOptsOrExit(argc, argv, verbose)};
  for(File& file : iterateFiles(opts)) {
    
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
