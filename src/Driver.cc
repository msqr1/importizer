#include "Opts.hpp"
#include <cstdlib>
#include <fmt/base.h>
#include <memory>
#include "clang/Tooling/Tooling.h"
#include "clang/Frontend/FrontendActions.h"

using namespace clang;
int main(const int argc, const char **argv) {
  try {
    assert(tooling::runToolOnCode(std::make_unique<SyntaxOnlyAction>(), "class X {};"));
    Opts opts{getOpts(argc, argv)};
  } catch (int code) {
    return code;
  }
  return EXIT_SUCCESS;
}
