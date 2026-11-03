// Diagnostics utility functions

#pragma once
#include <cassert>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/FormatVariadic.h>
#include <llvm/Support/WithColor.h>
#include <llvm/Support/raw_ostream.h>
#include <system_error>

// Used for calling into API that already logged the error
class EmptyErr : public llvm::ErrorInfo<EmptyErr> {
public:
  static char ID; // NOLINT

  EmptyErr();
  void log(llvm::raw_ostream &OS) const noexcept override;
  [[nodiscard]] auto convertToErrorCode() const noexcept -> std::error_code override;
};

struct LogOpts {
  mutable const char *prog;
  mutable llvm::raw_ostream *stream;
} const extern globalLogOpts;

void setGlobalLogOpts(const LogOpts &opts);

// Log an error to the global output stream
template <bool period = true, typename... Ts> void err(const char *fmt, Ts &&...args) noexcept {
  assert(globalLogOpts.stream != nullptr);
  llvm::WithColor::error(*globalLogOpts.stream, globalLogOpts.prog);
  llvm::WithColor stream{*globalLogOpts.stream, llvm::raw_ostream::Colors::SAVEDCOLOR, true};
  stream << llvm::formatv(fmt, std::forward<Ts>(args)...);
  if constexpr (period) {
    stream << ".\n";
  } else {
    stream << '\n';
  }
}

// Log a warning to the global output stream
template <bool period = true, typename... Ts> void warn(const char *fmt, Ts &&...args) noexcept {
  assert(globalLogOpts.stream != nullptr);
  llvm::WithColor::warning(*globalLogOpts.stream, globalLogOpts.prog);
  llvm::WithColor stream{*globalLogOpts.stream, llvm::raw_ostream::Colors::SAVEDCOLOR, true};
  stream << llvm::formatv(fmt, std::forward<Ts>(args)...);
  if constexpr (period) {
    stream << ".\n";
  } else {
    stream << '\n';
  }
}
