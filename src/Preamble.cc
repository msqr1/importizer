#include "Preamble.hpp"
#include "OptProcessor.hpp"
#include "Directive.hpp"
#include "Minimizer.hpp"
#include "FileOp.hpp"
#include "Preprocessor.hpp"
#include <fmt/base.h>
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
void addStdImport(std::string& str, StdImportLvl lvl,
  const std::optional<std::string>& bootstrapStdModule) {
  if(lvl == StdImportLvl::StdCompat) {
    fmt::format_to(std::back_inserter(str),
      "import {}.compat;\n",
      bootstrapStdModule ? *bootstrapStdModule : "std");
  }
  else if(lvl == StdImportLvl::Std) {
    fmt::format_to(std::back_inserter(str),
      "import {};\n",
      bootstrapStdModule ? *bootstrapStdModule : "std");
  }
}
void handleInclude(const Opts& opts, Directive& include, const ResolveIncludeCtx& ctx,
  const File& file, StdImportLvl& lvl, std::string& imports, MinimizeCtx& sharedCtx,
  MinimizeCtx* transitionalIncludeCtx = nullptr) {
  const IncludeInfo& info{std::get<IncludeInfo>(include.extraInfo)};
  std::optional<StdIncludeType> stdIncludeType;
  auto replaceExtPush = [&]() {
    if(transitionalIncludeCtx == nullptr) return;
    include.str.replace(info.startOffset, info.includeStr.length(),
      fs::path(info.includeStr).replace_extension(opts.moduleInterfaceExt).string());
    transitionalIncludeCtx->emplace_back(std::move(include.str));
  };
  if(std::optional<fs::path> resolvedInclude{resolveInclude(ctx, info, file.path)}) {
    fs::path includePath{std::move(*resolvedInclude)};
    
    // Ignored headers
    for(const fs::path& p : opts.ignoredHdrs) {
      if(includePath == p) {
        sharedCtx.emplace_back(std::move(include.str));
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
    fmt::format_to(std::back_inserter(imports), "import {};\n",
      path2ModuleName(includePath));
    replaceExtPush();
  }
  else if(opts.stdIncludeToImport &&
    (stdIncludeType = getStdIncludeType(info.includeStr))) {
    if(lvl < StdImportLvl::StdCompat) {
      lvl = *stdIncludeType == StdIncludeType::CppOrCwrap ?
        StdImportLvl::Std : StdImportLvl::StdCompat;
    }
    if(transitionalIncludeCtx != nullptr) {
      transitionalIncludeCtx->emplace_back(std::move(include.str));
    }
  }
  else sharedCtx.emplace_back(std::move(include.str));
}
std::string getDefaultPreamble(const Opts& opts, std::vector<Directive>& directives,
  const File& file, bool& manualExport) {
  const ResolveIncludeCtx ctx{opts.inDir, opts.includePaths};
  std::string preamble;
  std::string imports;
  StdImportLvl lvl{StdImportLvl::Unused};

  // Only convert include to import for source with a main(), paired or unpaired
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
    MinimizeCtx GMFCtx;
    for(Directive& directive : directives) switch(directive.type) {
    case DirectiveType::Include:
      handleInclude(opts, directive, ctx, file, lvl, imports, GMFCtx); 
      break;
    case DirectiveType::IfCond:
    case DirectiveType::Else:
    case DirectiveType::ElCond:
    case DirectiveType::EndIf:
    case DirectiveType::Define:
    case DirectiveType::Undef:
      GMFCtx.emplace_back(std::move(directive));
      break;
    default:
      GMFCtx.emplace_back(std::move(directive.str));
    }
    fmt::format_to(std::back_inserter(preamble),
      "module;\n"
      "{}",
      minimizeToStr(GMFCtx));

    // Convert header and unpaired source into module interface unit. Without
    // the "export " the file is a module implementation unit
    if(file.type == FileType::Hdr || file.type == FileType::UnpairedSrc)  {
      manualExport = true;
      preamble += "export ";
    }
    fmt::format_to(std::back_inserter(preamble),
      "module {};\n",
      path2ModuleName(file.relPath));
  }
  preamble += imports;
  addStdImport(preamble, lvl, opts.bootstrapStdModule);
  return preamble;
}
std::string getTransitionalPreamble(const Opts& opts,
  std::vector<Directive>& directives, const File& file, bool& manualExport) {
  const ResolveIncludeCtx ctx{opts.inDir, opts.includePaths};
  std::string preamble;
  MinimizeCtx sharedCtx;
  MinimizeCtx includeCtx;
  std::string moduleStr;
  StdImportLvl lvl{StdImportLvl::Unused};
  if(file.type == FileType::SrcWithMain) {
    for(Directive& directive : directives) switch(directive.type) {
    case DirectiveType::Include:
      handleInclude(opts, directive, ctx, file, lvl, moduleStr, sharedCtx, &includeCtx);
      break;
    case DirectiveType::Define:
    case DirectiveType::Undef:
    case DirectiveType::IfCond:
    case DirectiveType::ElCond:
    case DirectiveType::Else:
    case DirectiveType::EndIf:
      includeCtx.emplace_back(directive);
      sharedCtx.emplace_back(std::move(directive));
      break;
    default:
      includeCtx.emplace_back(directive.str);
      sharedCtx.emplace_back(std::move(directive.str));
    }
  }

  // Convert to module interface/implementation
  else {

    // Convert header and unpaired source into module interface unit. Without
    // the "export " the file is a module implementation unit
    if(file.type == FileType::Hdr || file.type == FileType::UnpairedSrc)  {
      manualExport = true;
      moduleStr += "export ";
    }
    fmt::format_to(std::back_inserter(moduleStr),
      "module {};\n",
      path2ModuleName(file.relPath));
    for(Directive& directive : directives) switch(directive.type) {
    case DirectiveType::Include:
      handleInclude(opts, directive, ctx, file, lvl, moduleStr, sharedCtx, &includeCtx);
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
      includeCtx.emplace_back(directive);
      sharedCtx.emplace_back(std::move(directive));
      break;
    default:
      includeCtx.emplace_back(directive.str);
      sharedCtx.emplace_back(std::move(directive.str));
    }

    // generic_string() to convert '\' to '/'
    fmt::format_to(std::back_inserter(preamble),
      "#ifdef {}\n"
      "module;\n"
      "#endif\n"
      "#include \"{}\"\n",
      opts.transitionalOpts->mi_control,
      ("." / opts.transitionalOpts->exportMacrosPath)
      .lexically_relative("." / file.relPath.parent_path()).generic_string());
  }
  addStdImport(moduleStr, lvl, opts.bootstrapStdModule);
  fmt::format_to(std::back_inserter(preamble),
    "{}"
    "#ifdef {}\n"
    "{}"
    "#else\n"
    "{}"
    "#endif\n",
    minimizeToStr(sharedCtx), opts.transitionalOpts->mi_control,
    moduleStr, minimizeToStr(includeCtx));
  return preamble;
}

} // namespace

bool addPreamble(File& file, std::vector<Directive>&& directives, const Opts& opts) {
  bool manualExport{};
  fmt::format_to(std::inserter(file.content, file.content.begin()),
    "{}\n",
    opts.transitionalOpts ?
    getTransitionalPreamble(opts, directives, file, manualExport) :
    getDefaultPreamble(opts, directives, file, manualExport));
  return manualExport;
}