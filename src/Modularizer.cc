#include "Modularizer.hpp"
#include "Lexer.hpp"
#include "ArgProcessor.hpp"
#include "Base.hpp"
#include "FileOp.hpp"
#include <vector>
#include <algorithm>
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

bool modularize(File& file, const LexResult& lexRes, const Opts& opts) {
  bool manualExport{};
  GetIncludeCtx ctx{opts.inDir, opts.includePaths};
  std::optional<fs::path> maybeResolvedInclude;
  std::string fileStart;
  if(file.relPath == "Main.cc") file.type = File::Type::SrcWithMain;

  // Convert to module interface/implementation
  if(file.type == File::Type::Hdr || file.type != File::Type::SrcWithMain) {
    fileStart += "module;\n";
    std::string afterModuleDecl;
    for(const Directive& directive : lexRes.directives) {
      switch(directive.type) {
      case Directive::Type::Include: {
        size_t start{directive.str.find('<')};
        size_t end;
        bool isAngle{start != notFound};
        if(isAngle) {
          start++;
          end = directive.str.find('>', start);
        }
        else {
          start = directive.str.find('"') + 1;
          end = directive.str.find('"', start);
        }
        fs::path includePath{directive.str.substr(start, end - start)};
        maybeResolvedInclude = isAngle ? getAngleInclude(ctx, includePath) :
          getQuotedInclude(ctx, includePath, file.relPath);
        
        // Skip #include of paired header
        if(file.type == File::Type::PairedSrc) {
          includePath.replace_extension(opts.srcExt);
          if(includePath == file.relPath) continue;
          includePath.replace_extension(opts.hdrExt);
        }
        if(maybeResolvedInclude) {
          bool ignore{};

          // Ignore import conversion of ignored headers
          for(const fs::path& p : opts.ignoredHeaders) {
            if(*maybeResolvedInclude == p) {
              fileStart += directive.str;
              ignore = true;
            }
          }
          if(ignore) continue;
          afterModuleDecl += "import " + path2ModuleName(*maybeResolvedInclude) + ";\n";
        }
        else fileStart += directive.str;
        break;
      }
      case Directive::Type::Condition:
      case Directive::Type::EndIf:
        afterModuleDecl += directive.str;
        [[fallthrough]];
      case Directive::Type::Define:
      case Directive::Type::Undef:
        fileStart += directive.str;
        break;
      default:
      }
    }

    // Convert header and unpaired source into module interface unit. Without 
    // the "export " the file is a module implementation unit
    if(file.type == File::Type::Hdr || file.type == File::Type::UnpairedSrc)  {
      manualExport = true;
      fileStart += "export ";
    }
    fileStart += "module ";
    fileStart += path2ModuleName(file.relPath);
    fileStart += ";\n";
    fileStart += afterModuleDecl;
  }

  // Only convert include to import for source with a main(), paired or unpaired
  else for(const Directive& directive : lexRes.directives) {
    if(directive.type == Directive::Type::Include) {
      size_t start{directive.str.find('<')};
      size_t end;
      bool isAngle{};
      if(start != notFound) {
        isAngle = true;
        start++;
        end = directive.str.find('>', start);
      }
      else {
        start = directive.str.find('"') + 1;
        end = directive.str.find('"', start);
      }
      fs::path includePath{directive.str.substr(start, end - start)};
      maybeResolvedInclude = isAngle ? getAngleInclude(ctx, includePath) :
        getQuotedInclude(ctx, includePath, file.relPath);
      if(maybeResolvedInclude) {
        bool ignore{};

        // Don't include2import ignored headers, keep them as #include
        for(const fs::path& p : opts.ignoredHeaders) {
          if(*maybeResolvedInclude == p) {
            ignore = true;
            break;
          }
        }
        if(!ignore) {
          fileStart += "import " + path2ModuleName(*maybeResolvedInclude) + ";\n";
          continue;
        }
      }
    }
    fileStart += directive.str;
  }
  file.content.insert(0, fileStart);
  return manualExport;
}