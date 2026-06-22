#include "run-test/CmpDir.hh"
#include "utils/Fs.hh"
#include "utils/Log.hh"
#include <algorithm>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Twine.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/FileUtilities.h>
#include <llvm/Support/Path.h>
#include <string>
#include <utility>
#include <vector>

namespace pth = llvm::sys::path;

bool cmpDir(llvm::StringRef dir, llvm::StringRef ref) noexcept {
  bool res{true};
  std::string msg;
  llvm::SmallString<128> path;
  llvm::SmallString<128> refPath;
  llvm::SmallString<128> relPath;
  std::vector<llvm::SmallString<128>> refRelPaths;

  auto checkRef{[&](const fs::directory_entry &ent) {
    if (ent.type() != fs::file_type::regular_file) {
      return true;
    }
    refPath = ent.path();
    relPath = refPath;
    pth::replace_path_prefix(relPath, ref, "");
    path = relPath;
    pth::replace_path_prefix(path, "", dir);
    if (!fs::exists(path)) {
      err("Not found: {}", relPath);
      res = false;
      return true;
    }
    int diffRes{llvm::DiffFilesWithTolerance(path, refPath, 0, 0, &msg)};
    switch (diffRes) {
    case 0: // Matches
      break;
    case 1:
      err("Mismatched content for {}", relPath);
      res = false;
      break;
    case 2:
      err("Unable to diff {} and {}: {}", path, refPath, msg);
      break;
    }
    refRelPaths.emplace_back(std::move(relPath));
    return true;
  }};
  if (!iterateDir(ref, checkRef)) {
    return false;
  }

  auto checkDir{[&](const fs::directory_entry &ent) {
    relPath = ent.path();
    pth::replace_path_prefix(relPath, dir, "");
    if (std::ranges::find(refRelPaths, relPath) != refRelPaths.end()) {
      return true;
    }
    err("Unexpected file: {}", relPath);
    res = false;
    return true;
  }};
  if (!iterateDir(dir, checkDir)) {
    return false;
  }
  return res;
}
