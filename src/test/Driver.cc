#include "fmt/base.h"
#include "subprocess.h"
#include "utils/FileOp.hh"
#include "utils/Log.hh"
#include <array>
#include <cstdlib>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

void progPrefix() { fmt::print(stderr, "test: "); }

// test [importizer path] [testDir] [outDir]
int main(const int argc, const char **argv) {
  fs::path testDir{argv[2]};
  fs::path outDir{argv[3]};
  const std::string config{(testDir / "Config.toml").string()};
  std::array<const char *, 4> cmd{argv[1], config.c_str(), "-o", argv[3]};
  subprocess_s proc;
  int res{subprocess_create(cmd.data(),
                            subprocess_option_no_window |
                                subprocess_option_combined_stdout_stderr,
                            &proc)};
  if (res != 0) {
    err("importizer failed to start\n");
    return EXIT_FAILURE;
  }
  int exitCode;
  res = subprocess_join(&proc, &exitCode);
  if (res != 0) {
    err("Unable to wait for importizer\n");
    return EXIT_FAILURE;
  }
  std::string out;
  if (!readToStr(subprocess_stdout(&proc), out, "importizer stdout")) {
    return EXIT_FAILURE;
  };
  std::string ref;
  if (!readToStr(testDir / "RefCli.txt", ref)) {
    return EXIT_FAILURE;
  }
}
