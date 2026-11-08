#include "testing/Unit.hh"
#define DOCTEST_CONFIG_IMPLEMENT
#include "testing/Doctest.hh"
#include "utils/Diag.hh"
#include <cstdio>
#include <doctest/extensions/doctest_util.h>
#include <ios>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/raw_ostream.h>
#include <ostream>
#include <streambuf>
#include <string>
#include <utility>
#include <vector>

namespace {

// std::streambuf that forwards to a llvm::raw_ostream
class LLVMStreamBuf : public std::streambuf {
  llvm::raw_ostream &stream_;

  // Writing single char
  auto overflow(int c) noexcept -> int override {
    if (c != EOF) {
      stream_.write(static_cast<unsigned char>(c));
    }
    return c;
  }

  // Writing multiple chars
  auto xsputn(const char *str, std::streamsize n) noexcept -> std::streamsize override {
    stream_.write(str, static_cast<size_t>(n));
    return n;
  }

public:
  LLVMStreamBuf(llvm::raw_ostream &stream) : stream_{stream} {}
};

class LLVMOstream : private LLVMStreamBuf, public std::ostream {
public:
  LLVMOstream(llvm::raw_ostream &stream)
      : LLVMStreamBuf{stream}, std::ostream{static_cast<std::streambuf *>(this)} {}
};

} // namespace

auto ok(llvm::Error err) noexcept -> bool { return !llvm::errorToBool(std::move(err)); }

auto runUnitTest(const char *suiteName, const std::vector<std::string> &args) noexcept -> int {
  doctest::Context ctx;
  LLVMOstream doctestOut{*globalLogCfg.stream()};
  doctest::applyCommandLine(ctx, args);
  ctx.setOption("ni", true);
  ctx.setOption("ts", suiteName);
  ctx.setCout(&doctestOut);
  return ctx.run();
}
