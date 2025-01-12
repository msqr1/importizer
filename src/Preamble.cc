#include "Preamble.hpp"
#include "ArgProcessor.hpp"
#include "Base.hpp"
#include "Directive.hpp"
#include "FileOp.hpp"
#include "Preprocessor.hpp"
#include "StdInclude.hpp"
#include <cstddef>
#include <optional>
#include <string>
#include <utility>
#include <variant>
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
  Unused,
  Std,
  StdCompat
};

struct Skip {};
struct KeepAsInclude {};
struct Keep4Hdr {};
using IncludeHandleResult = std::variant<Skip, KeepAsInclude, std::string, Keep4Hdr>;
IncludeHandleResult handleInclude(const IncludeInfo& info, const GetIncludeCtx& ctx, 
  const File& file, const Opts& opts, StdImportLvl& lvl) {
  std::optional<fs::path> resolvedInclude{
    info.isAngle ?  getAngleInclude(ctx, info.includeStr) :
    getQuotedInclude(ctx, info.includeStr, file.relPath)
  };
  if(resolvedInclude) {
    fs::path includePath{std::move(*resolvedInclude)};

    // Skip include2import conversion of paired header
    if(file.type == FileType::PairedSrc) {
      includePath.replace_extension(opts.srcExt);
      if(includePath == file.relPath) return Skip{};
      includePath.replace_extension(opts.hdrExt);
    }

    // Don't include2import ignored headers, keep them as #include
    for(const fs::path& p : opts.ignoredHeaders) {
      if(includePath == p) return KeepAsInclude{};
    }
    return "import " + path2ModuleName(includePath) + ";\n";
  }
  std::optional<StdIncludeType> maybeStdInclude;
  if(opts.stdInclude2Import && (maybeStdInclude = getStdIncludeType(info.includeStr))) {
    if(lvl < StdImportLvl::StdCompat) {
      lvl = *maybeStdInclude == StdIncludeType::CppOrCwrap ?
        StdImportLvl::Std : StdImportLvl::StdCompat;
    }
    if(opts.transitionalOpts) return Keep4Hdr{};
    else return Skip{};
  }
  return KeepAsInclude{};
}

std::string getDefaultPreamble(const Opts& opts, const std::vector<Directive>& directives,
  const File& file, bool& manualExport) {
  std::string preamble;
  const GetIncludeCtx ctx{opts.inDir, opts.includePaths};
  IncludeHandleResult res;
  StdImportLvl lvl;

  // Only convert include to import for source with a main(), paired or unpaired
  if(file.type == FileType::SrcWithMain) {
    for(const Directive& directive : directives) {
      if(directive.type == DirectiveType::Include) {
        res = handleInclude(std::get<IncludeInfo>(directive.extraInfo), ctx, file, opts,
          lvl);
        switch(res.index()) {
        case 2:
          preamble += std::get<std::string>(res);
          [[fallthrough]];
        case 0:
          continue;
        case 1:
        }
      }
      preamble += directive.str;
    }
  }

  // Convert to module interface/implementation
  else {
    preamble += "module;\n";
    std::string imports;
    for(const Directive& directive : directives) {
      switch(directive.type) {
      case DirectiveType::Include: {
        res = handleInclude(std::get<IncludeInfo>(directive.extraInfo), ctx, file, opts,
          lvl);
        switch(res.index()) {
        case 2:
          imports += std::get<std::string>(res);
          [[fallthrough]];
        case 0:
        case 3:
          continue;
        case 1:
        }
        preamble += directive.str;
        break;
      }
      case DirectiveType::Ifndef:
      case DirectiveType::IfCond:
      case DirectiveType::ElCond:
      case DirectiveType::EndIf:
      case DirectiveType::Define:
      case DirectiveType::Undef:
        imports += directive.str;
        preamble += directive.str;
        break;
      default:
      }
    }

    // Convert header and unpaired source into module interface unit. Without 
    // the "export " the file is a module implementation unit
    if(file.type == FileType::Hdr || file.type == FileType::UnpairedSrc)  {
      manualExport = true;
      preamble += "export ";
    }
    preamble += "module ";
    preamble += path2ModuleName(file.relPath);
    preamble += ";\n";
    preamble += imports;
  }
  if(lvl == StdImportLvl::StdCompat) preamble += "import std.compat;\n";
  else if(lvl == StdImportLvl::Std) preamble += "import std;\n";
  return preamble;
}

std::string getTransitionalPreamble(const Opts& opts,
  const std::vector<Directive>& directives, const File& file, bool& manualExport) {
  const GetIncludeCtx ctx{opts.inDir, opts.includePaths};
  IncludeHandleResult res;
  std::string preamble;
  std::string includes;
  StdImportLvl lvl;
  if(file.type == FileType::SrcWithMain) {
    preamble += "#ifdef ";
    preamble += opts.transitionalOpts->mi_control;
    preamble += "\n";
    for(const Directive& directive : directives) {
      if(directive.type == DirectiveType::Include) {
        res = handleInclude(std::get<IncludeInfo>(directive.extraInfo), ctx, file, opts,
          lvl);
        switch(res.index()) {
        case 0:
          continue;
        case 2:
          preamble += std::get<std::string>(res);
          break;
        case 1:
          preamble += directive.str;
        }
      }
      includes += directive.str;
    }
  }

  // Convert to module interface/implementation
  else {
    std::string imports;
    std::string GMF{"module;\n"};
    for(const Directive& directive : directives) switch(directive.type) {
    case DirectiveType::Include: {
      res = handleInclude(std::get<IncludeInfo>(directive.extraInfo), ctx, file, opts, lvl);
      switch(res.index()) {
      case 1:
        includes += directive.str;
        GMF += directive.str;
        [[fallthrough]];
      case 0:
        break;
      case 2:
        imports += std::get<std::string>(res);
        [[fallthrough]];
      case 3:
        includes += directive.str;
      }
      break;
    }
    case DirectiveType::Ifndef:
      if(std::holds_alternative<IncludeGuard>(directive.extraInfo)) {
        preamble += directive.str;
        continue;
      }
      else {
        GMF += directive.str;
        imports += directive.str;
        includes += directive.str;
      }
    case DirectiveType::Define:
      if(std::holds_alternative<IncludeGuard>(directive.extraInfo)) {
        preamble += directive.str;
        preamble += "#include \"";

        // Fake the same root path
        preamble += ("." / opts.transitionalOpts->exportMacrosPath)
          .lexically_relative("." / file.relPath);
        preamble += "\"\n";
        continue;
      }
      [[fallthrough]];
    case DirectiveType::Undef:
    case DirectiveType::IfCond:
    case DirectiveType::ElCond:
    case DirectiveType::EndIf:
      GMF += directive.str;
      imports += directive.str;
      includes += directive.str;
      break;
    case DirectiveType::PragmaOnce:
      preamble += directive.str;
      preamble += "#include \"";

      // Fake the same root path
      preamble += ("." / opts.transitionalOpts->exportMacrosPath)
          .lexically_relative("." / file.relPath);
      preamble += "\"\n";
      break;
    case DirectiveType::Other:
    }
    preamble += "#ifdef ";
    preamble += opts.transitionalOpts->mi_control;
    preamble += "\n";
    preamble += GMF;

    // Convert header and unpaired source into module interface unit. Without 
    // the "export " the file is a module implementation unit
    if(file.type == FileType::Hdr || file.type == FileType::UnpairedSrc)  {
      manualExport = true;
      preamble += "export ";
    }
    preamble += "module ";
    preamble += path2ModuleName(file.relPath);
    preamble += ";\n";
    preamble += imports;
  }
  if(lvl == StdImportLvl::StdCompat) preamble += "import std.compat;\n";
  else if(lvl == StdImportLvl::Std) preamble += "import std;\n";
  preamble += "#else\n";
  preamble += includes;
  preamble += "#endif\n";
  return preamble;
}

}

bool insertPreamble(File& file, const std::vector<Directive>& directives, const Opts& opts) {
  logIfVerbose("Modularizing...");
  bool manualExport;
  file.content.insert(0, opts.transitionalOpts ?
    getTransitionalPreamble(opts, directives, file, manualExport) : 
    getDefaultPreamble(opts, directives, file, manualExport));
  return manualExport;
}