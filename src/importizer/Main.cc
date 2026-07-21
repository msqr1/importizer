#include "importizer/Main.hh"
#include "importizer/Opts.hh"
#include "utils/Fs.hh"
#include "utils/Glob.hh"
#include <clang/Tooling/JSONCompilationDatabase.h>
#include <cstdlib>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/GlobPattern.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/raw_ostream.h>
#include <queue>
#include <string>

namespace fs = llvm::sys::fs;
namespace pth = llvm::sys::path;

int importizerMain(const int argc, const char *const *argv) {
  Opts opts;
  if (!getOpts(argc, argv, opts)) {
    return EXIT_FAILURE;
  }
  std::queue<std::string> files;
  switch (opts.fileHelper.index()) {
  case 0: { // std::unique_ptr<tl::JSONCompilationDatabase>
    files.push_range(std::get<0>(opts.fileHelper)->getAllFiles());
    break;
  }
  case 1: { // Bootstrap
    const Bootstrap &b{std::get<1>(opts.fileHelper)};
    llvm::StringRef path;
    auto checkInDir{[&](const fs::directory_entry &ent) {
      path = ent.path();
      for (const Glob &g : b.globs) {
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
  }
  while (!files.empty()) {
    llvm::outs() << files.front();
    files.pop();
  }
  return EXIT_SUCCESS;
}
