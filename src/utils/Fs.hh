#pragma once
#include "utils/Log.hh"
#include <cassert>
#include <llvm/ADT/STLFunctionalExtras.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <string>
#include <system_error>

namespace fs = llvm::sys::fs;

namespace llvm {
class Twine;
class StringRef;
} // namespace llvm
namespace {

template <typename It>
[[nodiscard]] bool iterateDirImpl(
    const llvm::Twine &dir,
    llvm::function_ref<int(const fs::directory_entry &)> fn) noexcept {
  std::error_code ec;
  It it{dir, ec}, end;
  if (ec) {
    return err("Error iterating {}: {}", dir, ec.message());
  }
  while (it != end) {
    switch (fn(*it)) {
    case 0:
      break;
    case 1:
      return true;
    case 2:
      return false;
    default:
      assert(false);
    }
    it.increment(ec);
    if (ec) {
      return err("Error iterating {}: {}", dir, ec.message());
    }
  }
  return true;
}

} // namespace

// Nicer LLVM's directory iterator. fn returns values:
// 0 - Keep going
// 1 - Stop and return true from iterateDir (ok stop)
// 2 - Stop and return false from iterateDir (errorneous stop)
template <bool recurse = true>
[[nodiscard]] bool
iterateDir(const llvm::Twine &dir,
           llvm::function_ref<int(const fs::directory_entry &)> fn) noexcept {
  if constexpr (recurse) {
    return iterateDirImpl<fs::recursive_directory_iterator>(dir, fn);
  } else {
    return iterateDirImpl<fs::directory_iterator>(dir, fn);
  }
}

// Resolve a relative path from a given directory
void mkRelative(std::string &path, const llvm::Twine &dir) noexcept;

// Copy the contents of from into to. Returns true if successful.
[[nodiscard]] bool copyDir(const llvm::Twine &from,
                           const llvm::Twine &to) noexcept;

// Compare 2 files, returns values are:
// 0 - file1 & file2 has the same content
// 1 - file1 & file2 has different content
// 2 - Error occured
[[nodiscard]] int cmpFile(const llvm::Twine &file1,
                          const llvm::Twine &file2) noexcept;

// Compare 2 directories and print mismatches, returns value are:
// 0 - dir & ref is exactly the same (including file content)
// 1 - Any kind of mismatch (missing & unexpected files, unequal file content)
// 2 - Error occured
[[nodiscard]] int cmpDir(const llvm::Twine &dir,
                         const llvm::Twine &ref) noexcept;
