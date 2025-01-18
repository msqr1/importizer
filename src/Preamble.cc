#include "Preamble.hpp"
#include "OptProcessor.hpp"
#include "Base.hpp"
#include "Directive.hpp"
#include "FileOp.hpp"
#include "Preprocessor.hpp"
#include <fmt/base.h>
#include <fmt/std.h>
#include <cstddef>
#include <iterator>
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
  if(opts.stdIncludeToImport && (maybeStdInclude = getStdIncludeType(info.includeStr))) {
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
  StdImportLvl lvl{StdImportLvl::Unused};

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
        case 1:;
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
        case 1:;
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
      default:;
      }
    }

    // Convert header and unpaired source into module interface unit. Without 
    // the "export " the file is a module implementation unit
    if(file.type == FileType::Hdr || file.type == FileType::UnpairedSrc)  {
      manualExport = true;
      preamble += "export ";
    }
    fmt::format_to(std::back_inserter(preamble),
      "module {};\n"
      "{}",
      path2ModuleName(file.relPath), imports);
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
  StdImportLvl lvl{StdImportLvl::Unused};
  if(file.type == FileType::SrcWithMain) {
    fmt::format_to(std::back_inserter(preamble), "#ifdef {}\n",
      opts.transitionalOpts->mi_control);
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
          break;
        case 3:;
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
        break;
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
      [[fallthrough]];
    case DirectiveType::Define:
      if(std::holds_alternative<IncludeGuard>(directive.extraInfo)) {
        preamble += directive.str;
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
      break;
    case DirectiveType::Other:;
    }
    fmt::format_to(std::back_inserter(preamble),
      "#include \"{}\"\n"
      "#ifdef {}\n"
      "{}",
      ("." / opts.transitionalOpts->exportMacrosPath)
      .lexically_relative("." / file.relPath), 
      opts.transitionalOpts->mi_control, GMF);

    // Convert header and unpaired source into module interface unit. Without 
    // the "export " the file is a module implementation unit
    if(file.type == FileType::Hdr || file.type == FileType::UnpairedSrc)  {
      manualExport = true;
      preamble += "export ";
    }
    fmt::format_to(std::back_inserter(preamble), 
      "module {};\n"
      "{}",
      path2ModuleName(file.relPath), imports);
  }
  if(lvl == StdImportLvl::StdCompat) preamble += "import std.compat;\n";
  else if(lvl == StdImportLvl::Std) preamble += "import std;\n";
  fmt::format_to(std::back_inserter(preamble),
    "#else\n{}"
    "#endif\n"
    , includes);
  return preamble;
}

} // namespace

bool insertPreamble(File& file, const std::vector<Directive>& directives, const Opts& opts) {
  bool manualExport{};
  file.content.insert(0, opts.transitionalOpts ?
    getTransitionalPreamble(opts, directives, file, manualExport) : 
    getDefaultPreamble(opts, directives, file, manualExport));
  return manualExport;
}