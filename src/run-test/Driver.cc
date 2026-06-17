#include "importizer/Entry.hh"
#include "run-test/CmpDir.hh"
#include "utils/Log.hh"
#include <array>
#include <cassert>
#include <cstdlib>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Twine.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/FormatVariadic.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/raw_ostream.h>

namespace fs = llvm::sys::fs;

// run-test [testDir] [outDir]
// Expecting absolute paths & no input validation
int main(const int argc, const char *const *argv) {
  assert(argc == 3);

  // Always set program and log target before everything
  LogOpts selfLogOpts;
  logOpts = &selfLogOpts;
  selfLogOpts.prog = "run-test";
  selfLogOpts.target = &llvm::errs();

  llvm::SmallString<128> tmp;
  llvm::Twine testDir{argv[1]};
  (testDir + "/Config.toml").toStringRef(tmp);

  // Make sure argv strings are always null-terminated
  std::array<const char *, 4> cmd{
      "importizer",
      tmp.c_str(),
      "-o",
      argv[2],
  };
  llvm::SmallString<128> out{};
  llvm::raw_svector_ostream outStream{out};
  LogOpts importizerLogOpts{"importizer", &outStream};
  logOpts = &importizerLogOpts;
  const int rtn{entry(cmd.size(), cmd.data())};
  logOpts = &selfLogOpts;

  llvm::Twine refCli{testDir + "/RefCli.txt"};
  auto buf{llvm::MemoryBuffer::getFile(refCli, true)};
  if (!buf) {
    err("Unable to read {}: {}", refCli, buf.getError().message());
    return EXIT_FAILURE;
  }
  llvm::StringRef ref{buf->get()->getBuffer()};
  const int refRtn{ref.contains("importizer: error: ") ? EXIT_FAILURE
                                                       : EXIT_SUCCESS};

  bool errored{};
  if ((errored |= refRtn != rtn)) {
    err("Mismatched return code: expected {}, got {}", refRtn, rtn);
  }

  if ((errored |= out != ref)) {
    err("Mismatched CLI output, got:");

    // Don't format importizer's output
    *selfLogOpts.target << out;
  }

  (testDir + "/ref").toStringRef(tmp);
  errored |= fs::exists(tmp) && !cmpDir(argv[2], tmp);

  return errored ? EXIT_FAILURE : EXIT_SUCCESS;
}
