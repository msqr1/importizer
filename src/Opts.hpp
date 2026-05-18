#include <filesystem>
namespace fs = std::filesystem;
struct Opts {
  fs::path inDir;
  fs::path outDir;
};
Opts getOpts(const int argc, const char **argv);
