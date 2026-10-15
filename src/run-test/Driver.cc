#include "importizer/Main.hh"
#include "utils/Fs.hh"
#include "utils/Log.hh"
#include <cassert>
#include <cstdlib>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Twine.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/FormatVariadic.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/raw_ostream.h>

namespace fs = llvm::sys::fs;

// run-test [testDir] [root]
// Expecting absolute paths & there's no input validation
int main(const int argc, const char *const *argv) {
  assert(argc == 2 && "More CLI args than expected.");

  // Always set program & log stream before everything
  LogOpts logOpts;
  g_logOpts = &logOpts;
  logOpts.prog = "run-test";
  logOpts.stream = &llvm::errs();

  // CLI args
  const llvm::Twine testDir{argv[1]};
  const llvm::Twine root{argv[2]};

  llvm::SmallString<128> tmp;
  const llvm::Twine refDir{testDir + "/ref"};
  bool hasRef{fs::exists(refDir)};

  (testDir + "/Config.yml").toVector(tmp);

  // Make sure argv strings are always null-terminated
  llvm::SmallVector<const char *, 3> cmd{"importizer", tmp.c_str()};
  if (hasRef) {
    cmd.emplace_back("-w");
  }

  llvm::SmallString<128> out{};
  llvm::raw_svector_ostream outStream{out};
  LogOpts importizerLogOpts{"importizer", &outStream};
  g_logOpts = &importizerLogOpts;
  const int rtn{importizerMain(static_cast<int>(cmd.size()), cmd.data())};
  g_logOpts = &logOpts;

  const llvm::Twine refCliPath{testDir + "/RefCli.txt"};
  const auto buf{llvm::MemoryBuffer::getFile(refCliPath, false, false, false)};
  if (!buf) {
    err("Error reading {}: {}", refCliPath, buf.getError().message());
    return EXIT_FAILURE;
  }
  llvm::StringRef refCli{(**buf).getBuffer()};
  const int refRtn{refCli.contains(" error: ") ? EXIT_FAILURE : EXIT_SUCCESS};

  bool errored{};
  if ((errored |= refRtn != rtn)) {
    err("Mismatched return code: expected {}, got {}", refRtn, rtn);
  }

  if ((errored |= out != refCli)) {
    err<false>("Mismatched CLI output, got:");

    // Don't format importizer's output
    *logOpts.stream << out;
  }

  if (hasRef) {
    errored |= cmpDir(root, refDir) != CmpRes::Same;
  }

  return errored ? EXIT_FAILURE : EXIT_SUCCESS;
}
