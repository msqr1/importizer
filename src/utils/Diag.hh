// Diagnostics utility functions

#pragma once
#include <cassert>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/FormatVariadic.h>
#include <llvm/Support/WithColor.h>
#include <llvm/Support/raw_ostream.h>
#include <system_error>

// Signify that an error has occured, but no further action required. Usually used for calling into
// APIs that already logged their own errors.
class EmptyErr : public llvm::ErrorInfo<EmptyErr> {
public:
  static char ID; // NOLINT

  EmptyErr() = default;
  void log(llvm::raw_ostream &OS) const noexcept override;
  [[nodiscard]] auto convertToErrorCode() const noexcept -> std::error_code override;
};

struct LogOpts {
  mutable const char *prog;
  mutable llvm::raw_ostream *stream;
} const extern globalLogOpts;

void setGlobalLogOpts(const char *prog, llvm::raw_ostream *stream) noexcept;
void setGlobalLogOpts(const LogOpts &opts) noexcept;

// Log an error to the global output stream
template <typename... Ts> void logErr(const char *fmt, Ts &&...args) noexcept {
  assert(globalLogOpts.stream != nullptr && globalLogOpts.prog != nullptr);
  llvm::WithColor::error(*globalLogOpts.stream, globalLogOpts.prog);
  llvm::WithColor{*globalLogOpts.stream, llvm::raw_ostream::Colors::SAVEDCOLOR, true}
      << llvm::formatv(fmt, std::forward<Ts>(args)...) << '\n';
}

// Log a warning to the global output stream
template <typename... Ts> void logWarning(const char *fmt, Ts &&...args) noexcept {
  assert(globalLogOpts.stream != nullptr && globalLogOpts.prog != nullptr);
  llvm::WithColor::warning(*globalLogOpts.stream, globalLogOpts.prog);
  llvm::WithColor{*globalLogOpts.stream, llvm::raw_ostream::Colors::SAVEDCOLOR, true}
      << llvm::formatv(fmt, std::forward<Ts>(args)...) << '\n';
}

// Lambda class that just logs the error if it's not an EmptyErr.
class ErrorLogger {
public:
  // Adding noexcept fails llvm::handleError signature check.
  auto operator()(const llvm::ErrorInfoBase &errInfo) const -> llvm::Error;
};
