// Nicer way to work with vfs::InMemoryFileSystem for testing

#pragma once
#include <llvm/ADT/STLFunctionalExtras.h>
#include <llvm/Support/VirtualFileSystem.h>
#include <llvm/Support/raw_ostream.h>

namespace llvm {
class Twine;
} // namespace llvm

namespace vfs = llvm::vfs;

auto addDir(vfs::InMemoryFileSystem &memFS, const llvm::Twine &path) -> bool;
auto addFile1(vfs::InMemoryFileSystem &memFS, const llvm::Twine &path) -> bool;
auto addFile2(vfs::InMemoryFileSystem &memFS, const llvm::Twine &path) -> bool;

class InitableMemFS : public vfs::InMemoryFileSystem {
public:
  InitableMemFS(llvm::function_ref<void(vfs::InMemoryFileSystem &memFS)> initFn);
};
