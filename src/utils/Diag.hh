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
  static char ID;

  EmptyErr() = default;
  void log(llvm::raw_ostream &stream) const noexcept override;
  [[nodiscard]] auto convertToErrorCode() const noexcept -> std::error_code override;
};

class LogConfig {
  const char *prog_;
  llvm::raw_ostream *stream_;

public:
  LogConfig() = default;
  void set(const char *prog, llvm::raw_ostream *stream) noexcept;
  void set(const LogConfig &cfg) noexcept;
  [[nodiscard]] auto prog() const noexcept -> const char *;
  [[nodiscard]] auto stream() const noexcept -> llvm::raw_ostream *;
} extern globalLogCfg; // NOLINT

// Log an error to the global output stream
template <typename... Ts> void logErr(const char *fmt, Ts &&...args) noexcept {
  llvm::WithColor::error(*globalLogCfg.stream(), globalLogCfg.prog());
  llvm::WithColor{*globalLogCfg.stream(), llvm::raw_ostream::Colors::SAVEDCOLOR, true}
      << llvm::formatv(fmt, std::forward<Ts>(args)...) << '\n';
}

// Log a warning to the global output stream
template <typename... Ts> void logWarning(const char *fmt, Ts &&...args) noexcept {
  llvm::WithColor::warning(*globalLogCfg.stream(), globalLogCfg.prog());
  llvm::WithColor{*globalLogCfg.stream(), llvm::raw_ostream::Colors::SAVEDCOLOR, true}
      << llvm::formatv(fmt, std::forward<Ts>(args)...) << '\n';
}

// Lambda class that just logs the error if it's not an EmptyErr.
class ErrorLogger {
public:
  // Adding noexcept fails llvm::handleError signature check.
  auto operator()(const llvm::ErrorInfoBase &errInfo) const -> llvm::Error;
};
