#include "utils/Fs.hh"
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/Twine.h>
#include <llvm/Support/Path.h>
#include <string>

namespace pth = llvm::sys::path;

void mkRelative(std::string &path, const llvm::Twine &dir) {
  if (!pth::is_relative(path)) {
    return;
  }
  llvm::SmallString<128> tmp;
  dir.toVector(tmp);
  pth::append(tmp, path);
  path.assign(tmp.data(), tmp.size());
}
