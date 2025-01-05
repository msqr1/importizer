#include "Modularizer.hpp"
#include "Preprocessor.hpp"
#include "Directive.hpp"
#include "ArgProcessor.hpp"
#include "Base.hpp"
#include "FileOp.hpp"
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

}

bool modularize(File& file, const PreprocessResult& prcRes, const Opts& opts) {
  logIfVerbose("Modularizing...");
  bool manualExport{};
  GetIncludeCtx ctx{opts.inDir, opts.includePaths};
  std::optional<fs::path> maybeResolvedInclude;
  std::string fileStart;
  fs::path includePath;
  
  // Only convert include to import for source with a main(), paired or unpaired
  if(file.type == FileType::SrcWithMain) {
    for(const Directive& directive : prcRes.directives) {
      if(directive.type == DirectiveType::Include) {
        const IncludeInfo& info{std::get<IncludeInfo>(directive.info)};
        maybeResolvedInclude = info.isAngle ? getAngleInclude(ctx, info.includeStr) :
          getQuotedInclude(ctx, info.includeStr, file.relPath);
        
        if(maybeResolvedInclude) {
          includePath = std::move(*maybeResolvedInclude);

          // Skip include2import conversion of paired header
          if(file.type == FileType::PairedSrc) {
            includePath.replace_extension(opts.srcExt);
            if(includePath == file.relPath) continue;
            includePath.replace_extension(opts.hdrExt);
          }
          bool ignore{};

          // Don't include2import ignored headers, keep them as #include
          for(const fs::path& p : opts.ignoredHeaders) {
            if(includePath == p) {
              ignore = true;
              break;
            }
          }
          if(!ignore) {
            fileStart += "import " + path2ModuleName(includePath) + ";\n";
            continue;
          }
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
        IncludeInfo info{std::get<IncludeInfo>(directive.info)};
        maybeResolvedInclude = info.isAngle ? getAngleInclude(ctx, info.includeStr) :
          getQuotedInclude(ctx, info.includeStr, file.relPath);
        
        if(maybeResolvedInclude) {
          includePath = std::move(*maybeResolvedInclude);

          // Skip include2import conversion of paired header
          if(file.type == FileType::PairedSrc) {
            includePath.replace_extension(opts.srcExt);
            if(includePath == file.relPath) continue;
            includePath.replace_extension(opts.hdrExt);
          }
          bool ignore{};

          // Don't include2import ignored headers, keep them as #include
          for(const fs::path& p : opts.ignoredHeaders) {
            if(includePath == p) {
              fileStart += directive.str;
              ignore = true;
            }
          }
          if(ignore) continue;
          afterModuleDecl += "import " + path2ModuleName(includePath) + ";\n";
        }
        else fileStart += directive.str;
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
  
  file.content.insert(0, fileStart);
  return manualExport;
}