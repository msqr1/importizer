#include "Preamble.hpp"
#include "Util.hpp"
#include "OptProcessor.hpp"
#include "Directive.hpp"
#include "Minimizer.hpp"
#include "FileOp.hpp"
#include "Preprocessor.hpp"
#include <iterator>
#include <optional>
#include <string>
#include <string_view>
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
enum class StdImportLvl : char {
  Unused,
  Std,
  StdCompat
};
void addStdImport(std::string& str, StdImportLvl lvl) {
  if(lvl == StdImportLvl::StdCompat) str += "import std.compat;\n";
  else if(lvl == StdImportLvl::Std) str += "import std;\n";
}
void addModuleDecl(const File& file, bool exportRequired, std::string& moduleDecl) {

  // Convert header and unpaired source into module interface unit. Without
  // the "export " the file is a module implementation unit
  if(exportRequired) moduleDecl += "export ";
  formatTo(std::back_inserter(moduleDecl),
    "module {};\n",
    path2ModuleName(file.relPath));
}
void handleInclude(const Opts& opts, Directive& include, const ResolveIncludeCtx& ctx,
  const File& file, StdImportLvl& lvl, std::string& imports, MinimizeCtx& externalCtx,
  std::string* internalIncludes = nullptr) {
  const IncludeInfo& info{std::get<IncludeInfo>(include.extraInfo)};
  std::optional<StdIncludeType> stdIncludeType;
  auto replaceExtPush = [&]() {
    if(internalIncludes == nullptr) return;
    include.str.replace(info.startOffset, info.includeStr.length(),
      fs::path(info.includeStr).replace_extension(opts.moduleInterfaceExt).string());
    *internalIncludes += include.str;
  };
  
  // Treat computed include as external
  if(info.method == IncludeMethod::Computed) {
    externalCtx.emplace_back(std::move(include.str));
  }
  else if(std::optional<fs::path> resolvedInclude{resolveInclude(ctx, info, file.path)}) {
    fs::path includePath{std::move(*resolvedInclude)};
    
    // Ignored headers
    for(const fs::path& p : opts.ignoredHdrs) {
      if(includePath == p) {
        externalCtx.emplace_back(std::move(include.str));
        return;
      }
    }
    if(file.type == FileType::PairedSrc) {
      if(includePath.replace_extension(opts.srcExt) == file.relPath) {
        replaceExtPush();
        return;
      }
      includePath.replace_extension(opts.hdrExt);
    }
    if(file.type == FileType::UmbrellaHdr) imports += "export ";
    formatTo(std::back_inserter(imports), "import {};\n",
      path2ModuleName(includePath));
    replaceExtPush();
  }
  else if(opts.stdIncludeToImport &&
    (stdIncludeType = getStdIncludeType(info.includeStr))) {
    if(lvl < StdImportLvl::StdCompat) {
      lvl = *stdIncludeType == StdIncludeType::CppOrCwrap ?
        StdImportLvl::Std : StdImportLvl::StdCompat;
    }
    if(internalIncludes != nullptr) *internalIncludes += include.str;
  }
  else externalCtx.emplace_back(std::move(include.str));
}
std::string getDefaultPreamble(const Opts& opts, std::vector<Directive>& directives,
  const File& file, bool exportRequired) {
  const ResolveIncludeCtx ctx{opts.inDir, opts.includePaths};
  std::string preamble;
  std::string imports;
  StdImportLvl lvl{StdImportLvl::Unused};

  // Convert to module consumer
  if(file.type == FileType::SrcWithMain) {
    MinimizeCtx noImportCtx;
    for(Directive& directive : directives) switch(directive.type) {
    case DirectiveType::Include:
      handleInclude(opts, directive, ctx, file, lvl, imports, noImportCtx); 
      break;
    case DirectiveType::IfCond:
    case DirectiveType::ElCond:
    case DirectiveType::Else:
    case DirectiveType::EndIf:
    case DirectiveType::Define:
    case DirectiveType::Undef:
      noImportCtx.emplace_back(std::move(directive));
      break;
    default:
      noImportCtx.emplace_back(std::move(directive.str));
    }
    preamble = minimizeToStr(noImportCtx);
  }

  // Convert to module interface/implementation
  else {
    MinimizeCtx externalCtx;
    for(Directive& directive : directives) switch(directive.type) {
    case DirectiveType::Include:
      handleInclude(opts, directive, ctx, file, lvl, imports, externalCtx); 
      break;
    case DirectiveType::IfCond:
    case DirectiveType::Else:
    case DirectiveType::ElCond:
    case DirectiveType::EndIf:
    case DirectiveType::Define:
    case DirectiveType::Undef:
      externalCtx.emplace_back(std::move(directive));
      break;
    default:
      externalCtx.emplace_back(std::move(directive.str));
    }
    formatTo(std::back_inserter(preamble),
      "module;\n"
      "{}",
      minimizeToStr(externalCtx));
    addModuleDecl(file, exportRequired, preamble);
  }
  preamble += imports;
  addStdImport(preamble, lvl);
  return preamble;
}
std::string getTransitionalPreamble(const Opts& opts,
  std::vector<Directive>& directives, const File& file, bool exportRequired) {
  const ResolveIncludeCtx ctx{opts.inDir, opts.includePaths};
  std::string preamble;
  MinimizeCtx externalCtx;
  std::string internalIncludes;
  std::string moduleStr;
  StdImportLvl lvl{StdImportLvl::Unused};

  // Convert to module consumer
  if(file.type == FileType::SrcWithMain) {
    for(Directive& directive : directives) switch(directive.type) {
    case DirectiveType::Include:
      handleInclude(opts, directive, ctx, file, lvl, moduleStr, externalCtx, &internalIncludes);
      break;
    case DirectiveType::Define:
    case DirectiveType::Undef:
    case DirectiveType::IfCond:
    case DirectiveType::ElCond:
    case DirectiveType::Else:
    case DirectiveType::EndIf:
      externalCtx.emplace_back(std::move(directive));
      break; 
    default:
      unreachable();
    }
  }

  // Convert to module interface/implementation
  else {
    addModuleDecl(file, exportRequired, moduleStr); 
    for(Directive& directive : directives) switch(directive.type) {
    case DirectiveType::Include:
      handleInclude(opts, directive, ctx, file, lvl, moduleStr, externalCtx, &internalIncludes);
      break;
    case DirectiveType::PragmaOnce:
      preamble += directive.str;
      break;
    case DirectiveType::IfCond:
    case DirectiveType::Define:
      if(std::holds_alternative<IncludeGuard>(directive.extraInfo)) {
        preamble += directive.str;
        break;
      }
      [[fallthrough]];
    case DirectiveType::Undef:
    case DirectiveType::Else:
    case DirectiveType::ElCond:
    case DirectiveType::EndIf:
      externalCtx.emplace_back(std::move(directive));
      break;
    default:
      unreachable();
    }

    // generic_string() to convert '\' to '/'
    formatTo(std::back_inserter(preamble),
      "#ifdef {}\n"
      "module;\n"
      "#endif\n"
      "#include \"{}\"\n",
      opts.transitional->mi_control,
      ("." / opts.transitional->exportMacrosPath)
      .lexically_relative("." / file.relPath.parent_path()).generic_string());
  }
  addStdImport(moduleStr, lvl);
  formatTo(std::back_inserter(preamble),
    "{}"
    "#ifdef {}\n"
    "{}"
    "#else\n"
    "{}"
    "#endif\n",
    minimizeToStr(externalCtx), opts.transitional->mi_control,
    moduleStr, internalIncludes);
  return preamble;
}

} // namespace

void addPreamble(const Opts& opts, File& file, PreprocessRes&& res, bool exportRequired) {
  formatTo(std::inserter(file.content, file.content.begin() + res.insertionPos),
    "{:\n<{}}{}\n", 
    "", res.prefixNewlineCnt, opts.transitional ?
    getTransitionalPreamble(opts, res.directives, file, exportRequired) :
    getDefaultPreamble(opts, res.directives, file, exportRequired));
}