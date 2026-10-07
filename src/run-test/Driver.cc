#include "importizer/Main.hh"
#include "run-test/CmpDir.hh"
#include "utils/Log.hh"
#include <array>
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

// run-test [testDir] [outDir]
// Expecting absolute paths & there's no input validation
int main(const int, const char *const *argv) {
  // Always set program & log target before everything
  LogOpts logOpts;
  g_logOpts = &logOpts;
  logOpts.prog = "run-test";
  logOpts.target = &llvm::errs();

  llvm::SmallString<128> tmp;
  const llvm::Twine testDir{argv[1]};
  (testDir + "/ref").toVector(tmp);
  bool checkWrite{fs::exists(tmp)};

  (testDir + "/Config.yml").toVector(tmp);

  // Make sure argv strings are always null-terminated
  llvm::SmallVector<const char *, 3> cmd{"importizer", tmp.c_str()};
  if (checkWrite) {
    cmd.emplace_back("-w");
  }

  llvm::SmallString<128> out{};
  llvm::raw_svector_ostream outStream{out};
  LogOpts importizerLogOpts{"importizer", &outStream};
  g_logOpts = &importizerLogOpts;
  const int rtn{importizerMain(static_cast<int>(cmd.size()), cmd.data())};
  g_logOpts = &logOpts;

  const llvm::Twine refCli{testDir + "/RefCli.txt"};
  const auto buf{llvm::MemoryBuffer::getFile(refCli, true)};
  if (!buf) {
    err("Unable to read {}: {}", refCli, buf.getError().message());
    return EXIT_FAILURE;
  }
  llvm::StringRef ref{(**buf).getBuffer()};
  const int refRtn{ref.contains(" error: ") ? EXIT_FAILURE : EXIT_SUCCESS};

  bool errored{};
  if ((errored |= refRtn != rtn)) {
    err("Mismatched return code: expected {}, got {}", refRtn, rtn);
  }

  if ((errored |= out != ref)) {
    err<false>("Mismatched CLI output, got:");

    // Don't format importizer's output
    *logOpts.target << out;
  }

  errored |= checkWrite && !cmpDir(argv[2], tmp);

  return errored ? EXIT_FAILURE : EXIT_SUCCESS;
}
