#define DOCTEST_CONFIG_NO_WINDOWS_SEH
#define DOCTEST_CONFIG_NO_POSIX_SIGNALS
#define DOCTEST_CONFIG_NO_MULTITHREADING // CMake already parallelize
#define DOCTEST_CONFIG_IMPLEMENT
#include "testing/Doctest.hh"
#include <llvm/Support/Error.h>
#include <utility>

auto ok(bool res) noexcept -> bool { return res; }

auto ok(llvm::Error err) noexcept -> bool { return !llvm::errorToBool(std::move(err)); }
