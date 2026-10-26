#include "testing/E2E.hh"
#include "importizer/Main.hh"
#include "utils/Diag.hh"
#include "utils/FS.hh"
#include <cstdlib>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Twine.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/raw_ostream.h>

namespace fs = llvm::sys::fs;

auto runE2ETest(const llvm::Twine &testDir, const llvm::Twine &root) noexcept -> bool {
  // Always set program and log stream before everything
  LogOpts logOpts{.prog = "run-test e2e", .stream = &llvm::errs()};
  setGlobalLogOpts(logOpts);

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
  LogOpts importizerLogOpts{.prog = "importizer", .stream = &outStream};
  setGlobalLogOpts(importizerLogOpts);
  const int rtn{importizerMain(static_cast<int>(cmd.size()), cmd.data())};
  setGlobalLogOpts(logOpts);

  const llvm::Twine refCLIPath{testDir + "/RefCLI.txt"};
  const auto buf{llvm::MemoryBuffer::getFile(refCLIPath, false, false, false)};
  if (!buf) {
    return err("Error reading {}: {}", refCLIPath, buf.getError().message());
  }
  llvm::StringRef refCLI{(**buf).getBuffer()};
  const int refRtn{refCLI.contains(" error: ") ? EXIT_FAILURE : EXIT_SUCCESS};

  bool errored{refRtn != rtn};
  if (errored) {
    err("Mismatched return code: expected {}, got {}", refRtn, rtn);
  }
  errored |= out != refCLI;
  if (errored) {
    err<false>("Mismatched CLI output, got:");

    // Don't format importizer's output
    *logOpts.stream << out;
  }

  if (hasRef) {
    errored |= cmpDirs(root, refDir) != CmpRes::Same;
  }

  return !errored;
}
