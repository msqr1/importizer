#include "OptProcessor.hpp"
#include "Base.hpp"
#include <cmath>
#define ARGS_NOEXCEPT 1
#include "../3rdParty/args.hpp"
#define TOML_EXCEPTIONS 0
#define TOML_ENABLE_FORMATTERS 0
#include <toml++/toml.hpp>
#include <utility>
#include <vector>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace fs = std::filesystem;
namespace {

template <typename keyTp>
auto getTypeCk(const toml::table& tbl, std::string_view key) {
  const toml::node_view<const toml::node> node{tbl[key]};
  
  // TOML need std::string, not const char*, string_view, etc.
  if constexpr(std::is_convertible_v<keyTp, std::string>) {
    if(node.is_string()) return node.ref<std::string>();
  }
  else if(node.is<keyTp>()) return node.ref<keyTp>();

  // Clang++ < 20 can't handle [[noreturn]] in constructors (eg. exitWithErr).
#pragma GCC diagnostic ignored "-Wreturn-type"
#pragma GCC diagnostic push
  exitWithErr("Incorrect TOML type for {}", key);
#pragma GCC diagnostic pop
}
template<typename T>
auto getMustHave(const toml::table& tbl, std::string_view key) {
  if(!tbl.contains(key)) exitWithErr("{} must be specified", key);
  return getTypeCk<T>(tbl, key);
}

} // namespace
Opts getOptsOrExit(int argc, const char** argv) {
  Opts opts;
  args::ArgumentParser parser
    {"Backward compatibly convert header-based C++ code to modules"};
  args::HelpFlag help{parser, "", 
    "Print help and exit.",
    {'h', "help"}, args::Options::KickOut};
  args::HelpFlag version{parser, "", 
    "Print version and exit.", 
    {'v', "version"}, args::Options::KickOut};
  args::Group general{parser, "", args::Group::Validators::DontCare, args::Options::Global};
  args::Flag stdIncludeToImport{general, "",
   "Convert standard includes to 'import std' or 'import std.compat'.",
    {'s', "std-include-to-import"}, args::Options::Global};
  args::Flag logCurrentFile{general, "",
    "Print the current file being processed.",
    {'l', "log-current-file"}};
  args::Flag pragmaOnce{general, "",
    "Declare that you use '#pragma once' so importizer handles them.",
    {'p', "pragma-once"}};
  args::Flag SOFComments{general, "",
    "Declare that your files may start with comments (usually to specify a license) so "
    "importizer handles them. Note that it scans for the largest continuous SOF comment "
    "chain.",
    {'S', "SOF-comments"}};
  args::ValueFlag<std::string> includeGuard{general, "",
      "Declare that you use include guards so the tool handles them. You will provide a "
      "regex to match match the entire guard, for example: '[^\\s]+_HPP'.",
      {"include-guard"}};
  args::ValueFlag<std::string> config{general, "", 
    "Path to TOML configuration file (.toml), defaults to 'importizer.toml'.",
    {'c', "config"}};
  args::ValueFlag<std::string> inDir{general, "", 
    "Input directory (must be here or in the TOML file).",
    {'i', "in-dir"}};
  args::ValueFlag<std::string> outDir{general, "", 
    "Output directory (must be here or in the TOML file).",
    {'o', "out-dir"}};
  args::ValueFlag<std::string> hdrExt{general, "", 
    "Header file extension.",
    {"hdr-ext"}};
  args::ValueFlag<std::string> srcExt{general, "", 
    "Source (also module implementation unit) file extension.",
    {"src-ext"}};
  args::ValueFlag<std::string> moduleInterfaceExt{general, "", 
    "Module interface unit file extension.",
    {"module-interface-ext"}};
  args::ValueFlagList<std::string> includePaths{general, "",
    "Include paths searched when converting include to import.",
    {"include-paths"}};
  args::ValueFlagList<std::string> ignoredHdrs{general, "",
    "Paths relative to [inDir] of header files to ignore. Their paired sources, if "
    "available, will be treated as if they have a 'main()'.",
    {"ignored-hdrs"}};
  args::ValueFlagList<std::string> umbrellaHdrs{general, "",
    "Paths relative to [inDir] of modularized headers, but their 'import' are turned "
    "into 'export import'.",
    {"umbrella-hdrs"}};
  args::Group transitional{parser};
  args::Command transitionalCmd{transitional, "transitional",
    "Turn on transitional mode"};
  args::Flag backCompatHdrs{transitional, "",
    "Generate headers that include the module file to preserve #include for users. Note "
    "that in the project itself the module file is still included directly.",
    {'b', "back-compat-hdrs"}};
  args::ValueFlag<std::string> mi_control{transitional, "", 
    "Header-module switching macro identifier.",
    {"mi-control"}};
  args::ValueFlag<std::string> mi_exportKeyword{transitional, "", 
    "Exported symbol macro identifier.",
    {"mi-export-keyword"}};
  args::ValueFlag<std::string> mi_exportBlockBegin{transitional, "", 
    "Export block begin macro identifier.",
    {"mi-export-block-begin"}};
  args::ValueFlag<std::string> mi_exportBlockEnd{transitional, "", 
    "Export block end macro identifier.",
    {"mi-export-block-end"}};
  args::ValueFlag<std::string> exportMacrosPath{transitional, "", 
    "File path relative to outDir to store the export macros above.",
    {"export-macros-path"}};
  parser.ParseCLI(argc, argv);
  if(help) {
    print("{}", parser.Help());
    exitOK();
  }
  else if(version) {
    println("{}", "2.0.0");
    exitOK();
  }
  if(transitionalCmd) {
    println("Transitional");
  }
  print("{}", backCompatHdrs.Get());
  exitOK();
  return opts;
}
