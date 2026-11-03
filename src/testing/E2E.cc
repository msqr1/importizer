#include "testing/E2E.hh"
#include "importizer/Main.hh"
#include "utils/Diag.hh"
#include "utils/FS.hh"
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Twine.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/raw_ostream.h>
#include <utility>

namespace fs = llvm::sys::fs;

auto runE2ETest(const llvm::Twine &testDir, const llvm::Twine &root) noexcept -> llvm::Error {
  // Always set program and log stream before everything
  LogOpts logOpts{.prog = "test-tool e2e", .stream = &llvm::errs()};
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
  llvm::Error err{importizerMain(static_cast<int>(cmd.size()), cmd.data())};
  setGlobalLogOpts(logOpts);

  const llvm::Twine refCLIPath{testDir + "/RefCLI.txt"};
  const auto buf{llvm::MemoryBuffer::getFile(refCLIPath, false, false, false)};
  if (!buf) {
    return llvm::errorCodeToError(buf.getError());
  }
  llvm::StringRef refCLI{(**buf).getBuffer()};

  llvm::Error res{llvm::Error::success()};
  if (out != refCLI) {
    res = llvm::joinErrors(std::move(res),
                           llvm::createStringError("Unexpected CLI output, got:\n{}", out));
  }

  if (hasRef) {
    llvm::Expected<bool> cmpRes{cmpDirs(root, refDir)};
    if (!cmpRes) {
      return cmpRes.takeError();
    }
    if (!*cmpRes) {
      res = llvm::joinErrors(std::move(res), llvm::make_error<EmptyErr>());
    }
  }

  return res;
}
