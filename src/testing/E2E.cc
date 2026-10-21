#include "testing/E2E.hh"
#include "importizer/Main.hh"
#include "utils/Diag.hh"
#include "utils/Fs.hh"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"

namespace fs = llvm::sys::fs;

bool runE2ETest(const llvm::Twine &testDir, const llvm::Twine &root) {
  // Always set program and log stream before everything
  LogOpts logOpts;
  g_logOpts = &logOpts;
  logOpts.prog = "run-test e2e";
  logOpts.stream = &llvm::errs();

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
    return err("Error reading {}: {}", refCliPath, buf.getError().message());
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

  return errored;
}
