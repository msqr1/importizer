#include "utils/Diag.hh"
#include <llvm/Support/Error.h>
#include <llvm/Support/raw_ostream.h>
#include <cassert>
#include <string>
#include <system_error>

char EmptyErr::ID{};
void EmptyErr::log(llvm::raw_ostream & /*stream*/) const noexcept {}
auto EmptyErr::convertToErrorCode() const noexcept -> std::error_code {
  return llvm::inconvertibleErrorCode();
}

LogConfig globalLogCfg{}; // NOLINT

void LogConfig::set(char const *prog, llvm::raw_ostream *stream) noexcept {
  assert(prog != nullptr && stream != nullptr);
  prog_ = prog;
  stream_ = stream;
}

void LogConfig::set(LogConfig const &cfg) noexcept { set(cfg.prog_, cfg.stream_); }

auto LogConfig::prog() const noexcept -> char const * {
  assert(prog_ != nullptr);
  return prog_;
}

auto LogConfig::stream() const noexcept -> llvm::raw_ostream * {
  assert(stream_ != nullptr);
  return stream_;
}

auto ErrorLogger::operator()(llvm::ErrorInfoBase const &errInfo) const -> llvm::Error {
  if (errInfo.isA<EmptyErr>()) {
    return llvm::Error::success();
  }
  std::string const msg{errInfo.message()};
  assert(!msg.empty());
  logErr("{}", msg);
  return llvm::Error::success();
}

#if __has_feature(address_sanitizer)
extern "C" __attribute__((visibility("default"))) auto __asan_default_options() -> char const * {
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

// For Bionic CStdlib
[[gnu::used, gnu::retain]] alignas(64) thread_local char const TLS_ALIGNER{};
