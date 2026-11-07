#include "utils/Diag.hh"
#include <cassert>
#include <llvm/Support/Error.h>
#include <llvm/Support/raw_ostream.h>
#include <string>
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

auto ErrorLogger::operator()(const llvm::ErrorInfoBase &errInfo) const -> llvm::Error {
  if (errInfo.isA<EmptyErr>()) {
    return llvm::Error::success();
  }
  std::string msg{errInfo.message()};
  assert(!msg.empty());
  logErr("{}", msg);
  return llvm::Error::success();
}

#if __has_feature(address_sanitizer)
extern "C" __attribute__((visibility("default"))) auto __asan_default_options() -> const char * {
  return ":strict_string_checks=1"
#ifndef WIN32
         ":detect_leaks=1"
#endif
#ifndef __APPLE__
         ":check_initialization_order=1"
#endif
#ifdef __SANITIZER_DISABLE_CONTAINER_OVERFLOW__
         ":detect_container_overflow=0"
#endif
      ;
}
#endif

namespace {

// For Bionic CStdlib
[[gnu::used, gnu::retain]] alignas(64) const thread_local char tlsAligner{};

} // namespace
