#include "GMF.hpp"
#include "Directive.hpp"
#include "ArgProcessor.hpp"
#include "Base.hpp"
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

namespace {

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
std::optional<fs::path> getQuotedInclude(const GetIncludeCtx& ctx,  const fs::path& include, 
  const fs::path& currentFile) {
  fs::path p{currentFile};
  p.remove_filename();
  p /= include;
  if(fs::exists(p)) {
    p = fs::relative(p, ctx.inDir);
    if(p.native().find("..") == notFound) return p;
  }
  return getAngleInclude(ctx, include);
}

}

void insertGMF(std::string& code, const std::vector<Directive>& directives, const Opts& opts) {
  GetIncludeCtx ctx{opts.inDir, opts.includePaths};
  return;
}