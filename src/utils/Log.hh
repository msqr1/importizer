#pragma once
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/FormatVariadic.h>
#include <llvm/Support/WithColor.h>
#include <llvm/Support/raw_ostream.h>

struct LogOpts {
  llvm::StringRef prog;
  llvm::raw_ostream *target;
} extern *g_logOpts;

template <bool period = true, typename... Ts>
bool err(llvm::StringRef fmt, Ts &&...args) noexcept {
  llvm::WithColor::error(*g_logOpts->target, g_logOpts->prog);
  llvm::WithColor stream{*g_logOpts->target,
                         llvm::raw_ostream::Colors::SAVEDCOLOR, true};
  stream << llvm::formatv(fmt.data(), std::forward<Ts>(args)...);
  if constexpr (period) {
    stream << ".\n";
  } else {
    stream << '\n';
  }

  // So we can do return err(...); instead of err(...); return false;
  return false;
}

template <bool period = true, typename... Ts>
void warn(llvm::StringRef fmt, Ts &&...args) noexcept {
  llvm::WithColor::warning(*g_logOpts->target, g_logOpts->prog);
  llvm::WithColor stream{*g_logOpts->target,
                         llvm::raw_ostream::Colors::SAVEDCOLOR, true};
  stream << llvm::formatv(fmt.data(), std::forward<Ts>(args)...);
  if constexpr (period) {
    stream << ".\n";
  } else {
    stream << '\n';
  }
}
