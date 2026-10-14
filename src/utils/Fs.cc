#include "utils/Fs.hh"
#include "utils/Log.hh"
#include <cassert>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Twine.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/Path.h>
#include <string>
#include <system_error>
#include <vector>

namespace pth = llvm::sys::path;
namespace fs = llvm::sys::fs;

void mkRelative(std::string &path, const llvm::Twine &dir) noexcept {
  if (!pth::is_relative(path)) {
    return;
  }
  llvm::SmallString<128> tmp;
  dir.toVector(tmp);
  pth::append(tmp, path);
  path.assign(tmp.data(), tmp.size());
}

bool copyDir(const llvm::Twine &from, const llvm::Twine &to) noexcept {
  llvm::StringRef relPath;
  llvm::SmallString<0> tmp;
  from.toVector(tmp);
  auto copy{[&](const fs::directory_entry &ent) {
    relPath = ent.path();
    assert(relPath.starts_with(tmp));
    relPath = relPath.drop_front(tmp.size());
    llvm::Twine dst{to + relPath};
    fs::copy_file(ent.path(), dst);
    return 0;
  }};
  return iterateDir<true>(from, copy);
}

int cmpFile(const llvm::Twine &file1, const llvm::Twine &file2) noexcept {
  const auto buf1{llvm::MemoryBuffer::getFile(file1, false, false, false)};
  if (!buf1) {
    return errI("Error opening {}: {}", file1, buf1.getError().message());
  }
  const auto buf2{llvm::MemoryBuffer::getFile(file2, false, false, false)};
  if (!buf2) {
    return errI("Error opening {}: {}", file2, buf2.getError().message());
  }
  return (**buf1).getBuffer() == (**buf2).getBuffer();
}

int cmpDir(const llvm::Twine &dir, const llvm::Twine &ref) noexcept {
  int res{1};
  llvm::StringRef relPath;
  llvm::SmallString<0> tmp;
  ref.toVector(tmp);
  std::vector<std::string> refRelPaths;

  auto checkRef{[&](const fs::directory_entry &ent) {
    if (ent.type() != fs::file_type::regular_file) {
      return 0;
    }
    relPath = ent.path();
    assert(relPath.starts_with(tmp));
    relPath = relPath.drop_front(tmp.size());
    const llvm::Twine path{dir + relPath};
    if (!fs::exists(path)) {
      res = errI("Missing file: {}", relPath);
      return 0;
    }
    switch (cmpFile(path, ent.path())) {
    case 0:
      break;
    case 1:
      res = errI("Mismatched content for {}", relPath);
      break;
    case 2:
      return 2;
    }
    refRelPaths.emplace_back(relPath);
    return 0;
  }};
  if (!iterateDir(ref, checkRef)) {
    return 2;
  }

  dir.toVector(tmp);
  auto checkDir{[&](const fs::directory_entry &ent) {
    relPath = ent.path();
    assert(relPath.starts_with(tmp));
    relPath = relPath.drop_front(tmp.size());
    bool found{};
    for (llvm::StringRef refRelPath : refRelPaths) {
      if (refRelPath == relPath) {
        found = true;
        break;
      }
    }
    if (!found) {
      res = errI("Unexpected file: {}", relPath);
    }
    return true;
  }};
  if (!iterateDir(dir, checkDir)) {
    return 2;
  }
  return res - 1;
}
