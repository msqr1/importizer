#include "testing/E2E.hh"
#include "importizer/Main.hh"
#include "utils/Diag.hh"
#include "utils/FS.hh"
#include <array>
#include <cstdlib>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Twine.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/FormatVariadic.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/raw_ostream.h>
#include <system_error>
#include <utility>

namespace pth = llvm::sys::path;
namespace fs = llvm::sys::fs;

auto runE2ETest(const llvm::Twine &testDir, const llvm::Twine &outDir) noexcept -> llvm::Error {
  llvm::SmallString<64> refDir;
  pth::append(refDir, testDir, "ref");
  const bool hasRef{fs::exists(refDir)};

  llvm::SmallString<64> configPath;
  pth::append(configPath, testDir, "Config.yml");

  llvm::SmallString<64> outDirStr;
  outDir.toVector(outDirStr);

  // SmallString::c_str() guarantees null-termination
  llvm::SmallVector<const char *, 5> cmd{"importizer", configPath.c_str()};
  if (hasRef) {
    cmd.emplace_back("-w");
    cmd.emplace_back("-r");
    cmd.emplace_back(outDirStr.c_str());
    if (std::error_code err{fs::remove_directories(outDirStr)}) {
      return llvm::createStringError(
          err, llvm::formatv("When emptying directory {}: {}", outDirStr, err.message()));
    }
    if (std::error_code err{fs::create_directories(outDirStr)}) {
      return llvm::createStringError(
          err, llvm::formatv("When creating directory {}: {}", outDirStr, err.message()));
    }
    if (llvm::Error err{cpyDir(testDir + pth::get_separator() + "in", outDirStr)}) {
      return err;
    }
  }

  llvm::SmallString<64> out{};
  llvm::raw_svector_ostream outStream{out};
  LogConfig prevLogConfig{globalLogCfg};
  globalLogCfg.set("importizer", &outStream);
  importizerMain(static_cast<int>(cmd.size()), cmd.data());
  globalLogCfg.set(prevLogConfig);

  llvm::SmallString<64> refCLIPath;
  pth::append(refCLIPath, testDir, "RefCLI.txt");
  const auto buf{
      llvm::MemoryBuffer::getFile(refCLIPath, false, false, false),
  };
  if (std::error_code err{buf.getError()}) {
    return llvm::createStringError(
        err, llvm::formatv("When opening {}: {}.", refCLIPath, err.message()));
  }
  const llvm::StringRef refCLI{(**buf).getBuffer()};

  llvm::Error res{llvm::Error::success()};
  if (out != refCLI) {
    res = llvm::joinErrors(
        std::move(res),
        llvm::createStringError(llvm::inconvertibleErrorCode(),
                                llvm::formatv("Unexpected CLI output, got:\n{}", out)));
  }
  if (hasRef) {
    llvm::Expected<bool> cmpRes{cmpDirs(outDirStr, refDir)};
    if (!cmpRes) {
      return llvm::joinErrors(std::move(res), cmpRes.takeError());
    }
    if (!*cmpRes) {
      res = llvm::joinErrors(std::move(res), llvm::make_error<EmptyErr>());
    }
  }

  return res;
}

auto updateE2ETest(const llvm::Twine &testDir) noexcept -> llvm::Error {
  llvm::SmallString<64> refDir;
  pth::append(refDir, testDir, "ref");
  if (!fs::exists(refDir)) {
    return llvm::createStringError("Expected output directory not found");
  }

  llvm::SmallString<64> testDirStr;
  testDir.toVector(testDirStr);
  llvm::SmallString<64> tmpDir;
  if (std::error_code err{fs::createUniqueDirectory(pth::filename(testDirStr), tmpDir)}) {
    return llvm::createStringError(
        err, llvm::formatv("When creating directory {}: {}", tmpDir, err.message()));
  }
  llvm::SmallString<64> configPath;
  pth::append(configPath, testDir, "Config.yml");

  // Make sure argv strings are always null-terminated
  std::array<const char *, 5> cmd{"importizer", configPath.c_str(), "-w", "-r", tmpDir.c_str()};

  LogConfig prevLogConfig{globalLogCfg};
  globalLogCfg.set("importizer", prevLogConfig.stream());
  if (importizerMain(static_cast<int>(cmd.size()), cmd.data()) != EXIT_SUCCESS) {
    return llvm::make_error<EmptyErr>();
  }
  globalLogCfg.set(prevLogConfig);

  if (std::error_code err{fs::remove_directories(refDir)}) {
    return llvm::createStringError(
        err, llvm::formatv("When emptying directory {}: {}", refDir, err.message()));
  }
  if (std::error_code err{fs::create_directories(refDir)}) {
    return llvm::createStringError(
        err, llvm::formatv("When creating directory {}: {}", refDir, err.message()));
  }
  return cpyDir(tmpDir, refDir);
}
