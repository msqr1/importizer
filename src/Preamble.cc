#include "Preamble.hpp"
#include "OptProcessor.hpp"
#include "Directive.hpp"
#include "CondMinimizer.hpp"
#include "FileOp.hpp"
#include "Preprocessor.hpp"
#include <fmt/base.h>
#include <cstddef>
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
std::string replaceIncludeExt(Directive&& include, std::string_view moduleInterfaceExt) {
  const IncludeInfo& info{std::get<IncludeInfo>(include.extraInfo)};
  fs::path includePath{info.includeStr};
  includePath.replace_extension(moduleInterfaceExt);
  return include.str.replace(info.startOffset, info.includeStr.length(), includePath.string());
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
  std::optional<fs::path> resolvedInclude{
    info.isAngle ?  getAngleInclude(ctx, info.includeStr) :
    getQuotedInclude(ctx, info.includeStr, file.relPath)
  };
  if(resolvedInclude) {
    fs::path includePath{std::move(*resolvedInclude)};

    // Skip include to import conversion of paired header
    if(file.type == FileType::PairedSrc) {
      includePath.replace_extension(opts.srcExt);
      if(includePath == file.relPath) {
        if(opts.transitionalOpts) return ReplaceExtForPair{};
        return Skip{};
      }
      includePath.replace_extension(opts.hdrExt);
    }

    // Don't include to import ignored headers, keep them as #include
    for(const fs::path& p : opts.ignoredHeaders) {
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
  IncludeHandleResult res;
  StdImportLvl lvl{StdImportLvl::Unused};

  // Only convert include to import for source with a main(), paired or unpaired
  if(file.type == FileType::SrcWithMain) {
    MinimizeCondCtx mcCtx;
    for(Directive& directive : directives) switch(directive.type) {
    case DirectiveType::IfCond:
    case DirectiveType::ElCond:
    case DirectiveType::Else:
    case DirectiveType::EndIf:
      mcCtx.emplace_back(std::move(directive));
      break;
    case DirectiveType::Include:
      res = handleInclude(std::get<IncludeInfo>(directive.extraInfo), ctx, file, opts,
        lvl);

      // We're in default mode, no need to handle switch case 3 or 4
      switch(res.index()) {
      case 2: // ConvertToImport
        mcCtx.emplace_back(std::get<ConvertToImport>(std::move(res)));
        continue;
      case 0: // Skip
        continue;
      case 1:; // KeepAsInclude
      }
      [[fallthrough]];
    default:
      mcCtx.emplace_back(std::move(directive.str));
    }
    preamble = minimizeCondToStr(mcCtx);
  }

  // Convert to module interface/implementation
  else {
    MinimizeCondCtx includeCtx;
    MinimizeCondCtx importCtx;
    for(Directive& directive : directives) switch(directive.type) {
    case DirectiveType::IfCond:
    case DirectiveType::Else:
    case DirectiveType::ElCond:
    case DirectiveType::EndIf:
      includeCtx.emplace_back(directive);
      importCtx.emplace_back(std::move(directive));
      break;
    case DirectiveType::Include:
      res = handleInclude(std::get<IncludeInfo>(directive.extraInfo), ctx, file, opts,
        lvl);

      // We're in default mode, no need to handle switch case 3 or 4
      switch(res.index()) {
      case 0:
        break;
      case 2:
        importCtx.emplace_back(std::get<ConvertToImport>(std::move(res)));
        break;
      case 1:
        includeCtx.emplace_back(std::move(directive.str));
        break;
      }
      break;
    default:
      includeCtx.emplace_back(directive.str);
      importCtx.emplace_back(std::move(directive.str));
    }
    fmt::format_to(std::back_inserter(preamble),
      "module;\n"
      "{}",
      minimizeCondToStr(includeCtx));

    // Convert header and unpaired source into module interface unit. Without 
    // the "export " the file is a module implementation unit
    if(file.type == FileType::Hdr || file.type == FileType::UnpairedSrc)  {
      manualExport = true;
      preamble += "export ";
    }
    fmt::format_to(std::back_inserter(preamble),
      "module {};\n"
      "{}",
      path2ModuleName(file.relPath), minimizeCondToStr(importCtx));
  }
  if(lvl == StdImportLvl::StdCompat) preamble += "import std.compat;\n";
  else if(lvl == StdImportLvl::Std) preamble += "import std;\n";
  return preamble;
}
std::string getTransitionalPreamble(const Opts& opts,
  std::vector<Directive>& directives, const File& file, bool& manualExport) {
  const GetIncludeCtx ctx{opts.inDir, opts.includePaths};
  IncludeHandleResult res;
  std::string preamble;
  MinimizeCondCtx includeCtx;
  StdImportLvl lvl{StdImportLvl::Unused};
  if(file.type == FileType::SrcWithMain) {
    MinimizeCondCtx moduleCtx;
    for(Directive& directive : directives) switch(directive.type) {
    case DirectiveType::IfCond:
    case DirectiveType::ElCond:
    case DirectiveType::Else:
    case DirectiveType::EndIf:
      includeCtx.emplace_back(directive);
      moduleCtx.emplace_back(std::move(directive));
      break;
    case DirectiveType::Include:
      res = handleInclude(std::get<IncludeInfo>(directive.extraInfo), ctx, file, opts,
        lvl);

      switch(res.index()) {
      case 0: // Skip
        continue;
      case 2: // ConvertToImport
        includeCtx.emplace_back(
          replaceIncludeExt(std::move(directive), opts.moduleInterfaceExt));
        moduleCtx.emplace_back(std::get<ConvertToImport>(std::move(res)));
        continue;
      case 3: // KeepForHdr
        includeCtx.emplace_back(std::move(directive.str));
        continue;
      case 1:; // KeepAsInclude
      }
      [[fallthrough]];
    default:
      includeCtx.emplace_back(directive.str);
      moduleCtx.emplace_back(std::move(directive.str));
    }
    fmt::format_to(std::back_inserter(preamble), 
      "#ifdef {}\n"
      "{}",
      opts.transitionalOpts->mi_control, minimizeCondToStr(moduleCtx));
  }

  // Convert to module interface/implementation
  else {
    MinimizeCondCtx GMFCtx;
    MinimizeCondCtx importCtx;
    for(Directive& directive : directives) switch(directive.type) {
    case DirectiveType::IfCond:
      if(std::holds_alternative<IncludeGuard>(directive.extraInfo)) {
        preamble += directive.str;
        break;
      }
      [[fallthrough]];
    case DirectiveType::Else:
    case DirectiveType::ElCond:
    case DirectiveType::EndIf:
      includeCtx.emplace_back(directive);
      GMFCtx.emplace_back(directive);
      importCtx.emplace_back(std::move(directive));
      break;
    case DirectiveType::Include:
      res = handleInclude(std::get<IncludeInfo>(directive.extraInfo), ctx, file, opts, lvl);
      switch(res.index()) {
      case 0: // Skip
        break;
      case 1: // KeepAsInclude
        GMFCtx.emplace_back(directive.str);
        [[fallthrough]];
      case 3: // KeepForHdr
        includeCtx.emplace_back(std::move(directive.str));
        break;
      case 2: // ConvertToImport
        includeCtx.emplace_back(
          replaceIncludeExt(std::move(directive), opts.moduleInterfaceExt));
        importCtx.emplace_back(std::get<ConvertToImport>(std::move(res)));
        break;
      case 4: // ReplaceExtForPair
        includeCtx.emplace_back(
          replaceIncludeExt(std::move(directive), opts.moduleInterfaceExt));
      }
      break;
    case DirectiveType::PragmaOnce:
      preamble += directive.str;
      break;
    case DirectiveType::Define:
      if(std::holds_alternative<IncludeGuard>(directive.extraInfo)) {
        preamble += directive.str;
        break;
      }
      [[fallthrough]];
    default:
      includeCtx.emplace_back(directive.str);
      GMFCtx.emplace_back(directive.str);
      importCtx.emplace_back(std::move(directive.str));
    }

    // generic_string() to convert '\' to '/'
    fmt::format_to(std::back_inserter(preamble),
      "#include \"{}\"\n"
      "#ifdef {}\n"
      "module;\n"
      "{}",
      ("." / opts.transitionalOpts->exportMacrosPath)
      .lexically_relative("." / file.relPath).generic_string(),
      opts.transitionalOpts->mi_control, minimizeCondToStr(GMFCtx));

    // Convert header and unpaired source into module interface unit. Without 
    // the "export " the file is a module implementation unit
    if(file.type == FileType::Hdr || file.type == FileType::UnpairedSrc)  {
      manualExport = true;
      preamble += "export ";
    }
    fmt::format_to(std::back_inserter(preamble), 
      "module {};\n"
      "{}",
      path2ModuleName(file.relPath), minimizeCondToStr(importCtx));
  }

  // From here import section should be added, but not include section
  if(lvl == StdImportLvl::StdCompat) preamble += "import std.compat;\n";
  else if(lvl == StdImportLvl::Std) preamble += "import std;\n";
  fmt::format_to(std::back_inserter(preamble),
    "#else\n{}"
    "#endif\n"
    , minimizeCondToStr(includeCtx));
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