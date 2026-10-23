#include "testing/Unit.hh"
#include "utils/Diag.hh"
#include <cstdio>
#include <ios>
#include <ostream>
#include <streambuf>
#define DOCTEST_CONFIG_IMPLEMENT
#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#define DOCTEST_CONFIG_NO_EXCEPTIONS
#define DOCTEST_CONFIG_NO_WINDOWS_SEH
#define DOCTEST_CONFIG_NO_POSIX_SIGNALS
#include <doctest/doctest.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/raw_ostream.h>

namespace {

// std::streambuf that forwards to a llvm::raw_ostream
class LLVMStreamBuf : public std::streambuf {
  llvm::raw_ostream &stream;

  // Writing single char
  auto overflow(int c) noexcept -> int override {
    if (c != EOF) {
      stream.write(static_cast<unsigned char>(c));
    }
    return c;
  }

  // Writing multiple chars
  auto xsputn(const char *str, std::streamsize n) noexcept -> std::streamsize override {
    stream.write(str, static_cast<size_t>(n));
    return n;
  }

public:
  LLVMStreamBuf(llvm::raw_ostream &stream_) : stream{stream_} {}
};

class LLVMOstream : private LLVMStreamBuf, public std::ostream {
public:
  LLVMOstream(llvm::raw_ostream &stream)
      : LLVMStreamBuf{stream}, std::ostream{static_cast<std::streambuf *>(this)} {}
};

} // namespace

auto runUnitTest(const char *suiteName) noexcept -> int {
  // Always set program and log stream before everything.
  // In this case, since we only care about return values, we void all logs.
  LogOpts logOpts{.prog = "", .stream = &llvm::nulls()};
  setGlobalLogOpts(logOpts);

  doctest::Context ctx;
  LLVMOstream doctestOut{llvm::errs()};
  ctx.setOption("ts", suiteName);
  ctx.setCout(&doctestOut);
  const int doctestRtn{ctx.run()};
  return doctestRtn;
}
