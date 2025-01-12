#include "Preamble.hpp"
#include "ArgProcessor.hpp"
#include "Base.hpp"
#include "Directive.hpp"
#include "FileOp.hpp"
#include "Preprocessor.hpp"
#include "StdInclude.hpp"
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

namespace {

std::string path2ModuleName(const fs::path& inDirRel) {
  fs::path path{inDirRel};
  path.replace_extension();
  std::string pathStr{path.generic_string()};
  for(char& c : pathStr) {
    if(c == '/') c = '.';

    // We only convert existing filenames, so we are safe to assume
    // '-' is the only char allowed in filenames but not in module names
    else if(c == '-') c = '_';
  }
  return pathStr;
}

// Just for the sake of making the lines not irritatingly long
// Also these fixed from the start
struct GetIncludeCtx {
  const fs::path& inDir;
  const std::vector<fs::path>& includePaths;
  GetIncludeCtx(const fs::path& inDir, const std::vector<fs::path>& includePaths):
    inDir{inDir}, includePaths{includePaths} {}
};

// Returns a resolved include, the path of the include relative to inDir, 
// return std::nullopt when the include doesn't exist, or not under inDir
std::optional<fs::path> getAngleInclude(const GetIncludeCtx& ctx, const fs::path& include) {
  fs::path p;
  for(const fs::path& includePath : ctx.includePaths) {
    p = includePath / include;
    if(fs::exists(p)) {
      p = fs::relative(p, ctx.inDir);
      if(p.native().find("..") == notFound) return p;
    }
  };
  return std::nullopt;
}

std::optional<fs::path> getQuotedInclude(const GetIncludeCtx& ctx, const fs::path& include, 
  const fs::path& currentFile) {
  fs::path p{currentFile};
  p.remove_filename();
  p = ctx.inDir / p;
  p /= include;
  if(fs::exists(p)) {
    p = fs::relative(p, ctx.inDir);
    if(p.native().find("..") == notFound) return p;
  }
  return getAngleInclude(ctx, include);
}

enum class StdImportLvl : char {
  No,
  Std,
  StdCompat
};
enum class IncludeAction : char {
  Continue,
  KeepAsInclude,
};
IncludeAction handleInclude(const IncludeInfo& info, const GetIncludeCtx& ctx, 
  const File& file, const Opts& opts, std::string& imports, StdImportLvl& importStd) {
  std::optional<fs::path> maybeResolvedInclude{
    info.isAngle ?  getAngleInclude(ctx, info.includeStr) :
    getQuotedInclude(ctx, info.includeStr, file.relPath)
  };
  if(maybeResolvedInclude) {
    fs::path includePath{std::move(*maybeResolvedInclude)};

    // Skip include2import conversion of paired header
    if(file.type == FileType::PairedSrc) {
      includePath.replace_extension(opts.srcExt);
      if(includePath == file.relPath) return IncludeAction::Continue;
      includePath.replace_extension(opts.hdrExt);
    }

    // Don't include2import ignored headers, keep them as #include
    for(const fs::path& p : opts.ignoredHeaders) {
      if(includePath == p) return IncludeAction::KeepAsInclude;
    }
    imports += "import " + path2ModuleName(info.includeStr) + ";\n";
    return IncludeAction::Continue;
  }
  std::optional<StdIncludeType> maybeStdInclude;
  if(opts.stdInclude2Import && (maybeStdInclude = getStdIncludeType(info.includeStr))) {
    if(importStd < StdImportLvl::StdCompat) {
      importStd = *maybeStdInclude == StdIncludeType::CppOrCwrap ?
        StdImportLvl::Std : StdImportLvl::StdCompat;
    }
    return IncludeAction::Continue;
  }
  return IncludeAction::KeepAsInclude;
}

}

bool modularize(File& file, const PreprocessResult& prcRes, const Opts& opts) {
  logIfVerbose("Modularizing...");
  bool manualExport{};
  const GetIncludeCtx ctx{opts.inDir, opts.includePaths};
  std::string fileStart;

  file.content.insert(0, fileStart);
  return manualExport;
}