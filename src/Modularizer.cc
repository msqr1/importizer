#include "Modularizer.hpp"
#include "Preprocessor.hpp"
#include "Directive.hpp"
#include "ArgProcessor.hpp"
#include "Base.hpp"
#include "FileOp.hpp"
#include "StdInclude.hpp"
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

IncludeAction handleInclude(const IncludeInfo& info, const GetIncludeCtx& ctx, const File& file,
  const Opts& opts, std::string& imports, StdImportLvl& importStd) {
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
      if(*maybeStdInclude == StdIncludeType::CppOrCwrap) importStd = StdImportLvl::Std;
      else importStd = StdImportLvl::StdCompat;
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
  StdImportLvl importStd;

  // Only convert include to import for source with a main(), paired or unpaired
  if(file.type == FileType::SrcWithMain) {
    for(const Directive& directive : prcRes.directives) {
      if(directive.type == DirectiveType::Include) {
        const IncludeInfo info{std::get<IncludeInfo>(directive.info)};
        switch(handleInclude(info, ctx, file, opts, fileStart, importStd)) {
        case IncludeAction::Continue:
          continue;
        case IncludeAction::KeepAsInclude:
        }
      }
      fileStart += directive.str;
    }
  }

  // Convert to module interface/implementation
  else {
    fileStart += "module;\n";
    std::string afterModuleDecl;
    for(const Directive& directive : prcRes.directives) {
      switch(directive.type) {
      case DirectiveType::Include: {
        const IncludeInfo info{std::get<IncludeInfo>(directive.info)};
        switch(handleInclude(info, ctx, file, opts, afterModuleDecl, importStd)) {
        case IncludeAction::KeepAsInclude:
          fileStart += directive.str;
          [[fallthrough]];
        case IncludeAction::Continue:
          continue;
        }
        break;
      }
      case DirectiveType::Ifndef:
      case DirectiveType::IfCond:
      case DirectiveType::ElCond:
      case DirectiveType::EndIf:
        afterModuleDecl += directive.str;
        [[fallthrough]];
      case DirectiveType::Define:
      case DirectiveType::Undef:
        fileStart += directive.str;
        break;
      default:
      }
    }

    // Convert header and unpaired source into module interface unit. Without 
    // the "export " the file is a module implementation unit
    if(file.type == FileType::Hdr || file.type == FileType::UnpairedSrc)  {
      manualExport = true;
      fileStart += "export ";
    }
    fileStart += "module ";
    fileStart += path2ModuleName(file.relPath);
    fileStart += ";\n";
    fileStart += afterModuleDecl;
  }
  if(importStd == StdImportLvl::StdCompat) fileStart +="import std.compat;\n";
  else if(importStd == StdImportLvl::Std) fileStart += "import std;\n";
  file.content.insert(0, fileStart);
  return manualExport;
}