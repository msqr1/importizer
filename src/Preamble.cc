#include "Preamble.hpp"
#include "OptProcessor.hpp"
#include "Directive.hpp"
#include "Minimizer.hpp"
#include "FileOp.hpp"
#include "Preprocessor.hpp"
#include <fmt/base.h>
#include <cstdint>
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
std::string replaceIncludeExt(Directive&& include, const fs::path& moduleInterfaceExt) {
  const IncludeInfo& info{std::get<IncludeInfo>(include.extraInfo)};
  return include.str.replace(info.startOffset, info.includeStr.length(),
    fs::path(info.includeStr).replace_extension(moduleInterfaceExt).string());
}

// Just for the sake of making the lines not irritatingly long
// Also these fixed from the start
struct GetIncludeCtx {
  const fs::path& inDir;
  const std::vector<fs::path>& includePaths;
};

// Returns a resolved include, the path of the include relative to inDir,
// return std::nullopt when the include doesn't exist, or not under inDir
std::optional<fs::path> getAngleInclude(const GetIncludeCtx& ctx, const fs::path& include) {
  fs::path p;
  for(const fs::path& includePath : ctx.includePaths) {
    p = includePath / include;
    if(fs::exists(p)) {
      p = fs::relative(p, ctx.inDir);
      if(*p.begin() != "..") return p;
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
    if(*p.begin() != "..") return p;
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
using ConvertToImport = std::string;
struct KeepForHdr {};
struct ReplaceExtForPair {};
using IncludeHandleResult = std::variant<Skip, KeepAsInclude, ConvertToImport,
  KeepForHdr, ReplaceExtForPair>;
IncludeHandleResult handleInclude(const IncludeInfo& info, const GetIncludeCtx& ctx,
  const File& file, const Opts& opts, StdImportLvl& lvl) {
  if(std::optional<fs::path> resolvedInclude{info.isAngle ?
    getAngleInclude(ctx, info.includeStr) :
    getQuotedInclude(ctx, info.includeStr, file.relPath)}) {
    fs::path includePath{std::move(*resolvedInclude)};

    if(file.type == FileType::PairedSrc) {
      includePath.replace_extension(opts.srcExt);
      if(includePath == file.relPath) {
        if(opts.transitionalOpts) return ReplaceExtForPair{};

        // Skip include to import conversion of paired header
        return Skip{};
      }
      includePath.replace_extension(opts.hdrExt);
    }

    // Don't include to import ignored headers, keep them as #include
    for(const fs::path& p : opts.ignoredHdrs) {
      if(includePath == p) return KeepAsInclude{};
    }
    return "import " + path2ModuleName(includePath) + ";\n";
  }
  std::optional<StdIncludeType> maybeStdInclude;
  if(opts.stdIncludeToImport && (maybeStdInclude = getStdIncludeType(info.includeStr))) {
    if(lvl < StdImportLvl::StdCompat) {
      lvl = *maybeStdInclude == StdIncludeType::CppOrCwrap ?
        StdImportLvl::Std : StdImportLvl::StdCompat;
    }
    if(opts.transitionalOpts) return KeepForHdr{};
    return Skip{};
  }
  return KeepAsInclude{};
}
std::string getDefaultPreamble(const Opts& opts, std::vector<Directive>& directives,
  const File& file, bool& manualExport) {
  const GetIncludeCtx ctx{opts.inDir, opts.includePaths};
  std::string preamble;
  std::string imports;
  IncludeHandleResult res;
  StdImportLvl lvl{StdImportLvl::Unused};

  // Only convert include to import for source with a main(), paired or unpaired
  if(file.type == FileType::SrcWithMain) {
    MinimizeCtx noImportCtx;
    for(Directive& directive : directives) switch(directive.type) {
    case DirectiveType::Include:
      res = handleInclude(std::get<IncludeInfo>(directive.extraInfo), ctx, file, opts,
        lvl);

      // We're in default mode, no need to handle switch case 3 or 4
      switch(res.index()) {
      case 2: // ConvertToImport
        imports += std::get<ConvertToImport>(res);
        continue;
      case 0: // Skip
        continue;
      case 1:; // KeepAsInclude
      }
      [[fallthrough]];
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
      res = handleInclude(std::get<IncludeInfo>(directive.extraInfo), ctx, file, opts,
        lvl);

      // We're in default mode, no need to handle switch case 3 or 4
      switch(res.index()) {
      case 0: // Skip
        break;
      case 2: // ConvertToImport
        imports += std::get<ConvertToImport>(std::move(res));
        break;
      case 1: // KeepAsInclude
        GMFCtx.emplace_back(std::move(directive.str));
      }
      break;
    case DirectiveType::IfCond:
    case DirectiveType::Else:
    case DirectiveType::ElCond:
    case DirectiveType::EndIf:
    case DirectiveType::Define:
    case DirectiveType::Undef:
      GMFCtx.emplace_back((std::move(directive)));
      break;
    default:
      GMFCtx.emplace_back(directive.str);
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
  if(lvl == StdImportLvl::StdCompat) preamble += "import std.compat;\n";
  else if(lvl == StdImportLvl::Std) preamble += "import std;\n";
  return preamble;
}
std::string getTransitionalPreamble(const Opts& opts,
  std::vector<Directive>& directives, const File& file, bool& manualExport) {
  const GetIncludeCtx ctx{opts.inDir, opts.includePaths};
  IncludeHandleResult res;
  std::string preamble;
  MinimizeCtx commonCtx;
  MinimizeCtx includeCtx;
  std::string moduleStr;
  StdImportLvl lvl{StdImportLvl::Unused};
  if(file.type == FileType::SrcWithMain) {
    for(Directive& directive : directives) switch(directive.type) {
    case DirectiveType::Include:
      res = handleInclude(std::get<IncludeInfo>(directive.extraInfo), ctx, file, opts,
        lvl);

      switch(res.index()) {
      case 0: // Skip
        break;
      case 1:; // KeepAsInclude
        commonCtx.emplace_back(std::move(directive.str));
        break;
      case 2: // ConvertToImport
        includeCtx.emplace_back(
          replaceIncludeExt(std::move(directive), opts.moduleInterfaceExt));
        moduleStr += std::get<ConvertToImport>(res);
        break;  
      case 3: // KeepForHdr
        includeCtx.emplace_back(std::move(directive.str));
      }
      break;
    case DirectiveType::Define:
    case DirectiveType::Undef:
    case DirectiveType::IfCond:
    case DirectiveType::ElCond:
    case DirectiveType::Else:
    case DirectiveType::EndIf:
      includeCtx.emplace_back(directive);
      commonCtx.emplace_back(std::move(directive));
      break;
    default:
      includeCtx.emplace_back(directive.str);
      commonCtx.emplace_back(std::move(directive.str));
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
      res = handleInclude(std::get<IncludeInfo>(directive.extraInfo), ctx, file, opts, lvl);
      switch(res.index()) {
      case 0: // Skip
        break;
      case 1: // KeepAsInclude
        commonCtx.emplace_back(directive.str);
        break;
      case 2: // ConvertToImport
        includeCtx.emplace_back(
          replaceIncludeExt(std::move(directive), opts.moduleInterfaceExt));
        moduleStr += std::get<ConvertToImport>(res);
        break;
      case 3: // KeepForHdr
        includeCtx.emplace_back(std::move(directive.str));
        break;
      case 4: // ReplaceExtForPair
        includeCtx.emplace_back(
          replaceIncludeExt(std::move(directive), opts.moduleInterfaceExt));
      }
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
      commonCtx.emplace_back(std::move(directive));
      break;
    default:
      includeCtx.emplace_back(directive.str);
      commonCtx.emplace_back(std::move(directive.str));
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

  // From here import section should be added, but not include section
  if(lvl == StdImportLvl::StdCompat) moduleStr += "import std.compat;\n";
  else if(lvl == StdImportLvl::Std) moduleStr += "import std;\n";
  fmt::format_to(std::back_inserter(preamble),
    "{}"
    "#ifdef {}\n"
    "{}"
    "#else\n"
    "{}"
    "#endif\n",
    minimizeToStr(commonCtx), opts.transitionalOpts->mi_control,
    moduleStr, minimizeToStr(includeCtx));
  return preamble;
}

} // namespace

bool insertPreamble(File& file, std::vector<Directive>&& directives, const Opts& opts) {
  bool manualExport{};
  fmt::format_to(std::inserter(file.content, file.content.begin()),
    "{}\n",
    opts.transitionalOpts ?
    getTransitionalPreamble(opts, directives, file, manualExport) :
    getDefaultPreamble(opts, directives, file, manualExport));
  return manualExport;
}