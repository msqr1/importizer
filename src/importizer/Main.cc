#include "importizer/Main.hh"
#include "importizer/Opts.hh"
#include <clang/Tooling/JSONCompilationDatabase.h>
#include <cstdlib>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/raw_ostream.h>

int importizerMain(const int argc, const char *const *argv) {
  Opts opts;
  if (!getOpts(argc, argv, opts)) {
    return EXIT_FAILURE;
  }
  /*
  std::queue<std::string> files;
  switch (opts.fileHelper.index()) {
  case 0: { // std::unique_ptr<tl::JSONCompilationDatabase>
    files.push_range(std::get<0>(opts.fileHelper)->getAllFiles());
    break;
  }
  case 1: { // Explicit
    const Explicit &e{std::get<1>(opts.fileHelper)};
    llvm::StringRef path;
    auto checkInDir{[&](const fs::directory_entry &ent) {
      path = ent.path();
      for (const Glob &g : e.globs) {
        if (g.match(pth::filename(path))) {
          files.emplace(path);
          break;
        }
      }
      return true;
    }};
    if (!iterateDir(opts.inDir, checkInDir)) {
      return EXIT_FAILURE;
    }
    break;
  }
  */
  return EXIT_SUCCESS;
}
