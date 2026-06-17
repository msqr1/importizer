#include "utils/Log.hh"
#include <llvm/ADT/STLFunctionalExtras.h>
#include <llvm/ADT/Twine.h>
#include <llvm/Support/FileSystem.h>
#include <system_error>

namespace fs = llvm::sys::fs;

namespace detail {

template <typename It>
[[nodiscard]] bool
iterateDir(const llvm::Twine &dir,
           llvm::function_ref<bool(const fs::directory_entry &)> fn) noexcept {
  std::error_code ec;
  It it{dir, ec}, end;
  if (ec) {
    err("Unable to iterate {}: {}", dir, ec.message());
    return false;
  }
  while (it != end) {
    it.increment(ec);
    if (ec) {
      err("Unable to iterate {}: {}", dir, ec.message());
      return false;
    }
    if (!fn(*it)) {
      return false;
    }
  }
  return true;
}

} // namespace detail

// Nicer LLVM's directory iterator. fn should return true to continue iterating.
template <bool recurse = true>
[[nodiscard]] bool
iterateDir(const llvm::Twine &dir,
           llvm::function_ref<bool(const fs::directory_entry &)> fn) noexcept {
  if constexpr (recurse) {
    return detail::iterateDir<fs::recursive_directory_iterator>(dir, fn);
  } else {
    return detail::iterateDir<fs::directory_iterator>(dir, fn);
  }
}
