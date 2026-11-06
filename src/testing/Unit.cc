#include "testing/Unit.hh"
#define DOCTEST_CONFIG_IMPLEMENT
#include "testing/Doctest.hh"
#include "utils/Diag.hh"
#include <cstdio>
#include <ios>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/raw_ostream.h>
#include <ostream>
#include <streambuf>
#include <utility>

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

auto ok(llvm::Error err) noexcept -> bool { return !llvm::errorToBool(std::move(err)); }

auto runUnitTest(const char *suiteName, const int argc, const char **argv) noexcept -> int {
  doctest::Context ctx{argc, argv};
  LLVMOstream doctestOut{*globalLogOpts.stream};
  ctx.setOption("ts", suiteName);
  ctx.setCout(&doctestOut);
  return ctx.run();
}
