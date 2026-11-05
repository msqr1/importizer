#include "utils/Diag.hh"
#include <cassert>
#include <llvm/Support/Error.h>
#include <llvm/Support/raw_ostream.h>
#include <system_error>

char EmptyErr::ID{};
void EmptyErr::log(llvm::raw_ostream & /*OS*/) const noexcept {}
auto EmptyErr::convertToErrorCode() const noexcept -> std::error_code {
  return llvm::inconvertibleErrorCode();
}

const LogOpts globalLogOpts{};

void setGlobalLogOpts(const char *prog, llvm::raw_ostream *stream) noexcept {
  assert(prog != nullptr && stream != nullptr);
  globalLogOpts.prog = prog;
  globalLogOpts.stream = stream;
}

void setGlobalLogOpts(const LogOpts &opts) noexcept { setGlobalLogOpts(opts.prog, opts.stream); }

#ifdef __has_feature
#if __has_feature(address_sanitizer)
extern "C" auto __asan_default_options() -> const char * {
  return ""
#if defined(__x86_64__) && defined(__APPLE__)
         // LSAN is only for amd64 Linux and MacOS, but it's only on by default
         // for Linux so we explicitly turn it on for MacOS
         ":detect_leaks=1"

#endif
      ;
}
#endif
#endif

namespace {

// For Bionic CStdlib
[[gnu::used, gnu::retain]] alignas(64) const thread_local char tlsAligner{};

} // namespace
