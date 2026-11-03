#define DOCTEST_CONFIG_IMPLEMENT
#include "testing/Unit.hh"
#include "utils/Diag.hh"
#include <cstdio>
#include <ios>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/raw_ostream.h>
#include <ostream>
#include <streambuf>

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

auto runUnitTest(const char *suiteName, const int argc, const char *const *argv) noexcept -> int {
  // Always set program and log stream before everything.
  LogOpts logOpts{.prog = "", .stream = &llvm::errs()};
  setGlobalLogOpts(logOpts);

  doctest::Context ctx{argc, argv};
  LLVMOstream doctestOut{*logOpts.stream};
  ctx.setOption("ts", suiteName);
  ctx.setCout(&doctestOut);
  const int doctestRtn{ctx.run()};
  return doctestRtn;
}
