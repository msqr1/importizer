#include "importizer/Entry.hh"
#include "run-test/CmpDir.hh"
#include "utils/FileOp.hh"
#include "utils/Log.hh"
#include <array>
#include <cassert>
#include <cstdio> // IWYU pragma: keep for stderr
#include <cstdlib>
#include <filesystem>
#include <fmt/base.h>
#include <string>
#include <string_view>

namespace fs = std::filesystem;

// test [testDir] [outDir]
// Expecting absolute paths & no input validation
int main(const int argc, const char *const *argv) {
  assert(argc == 3);

  // Always set program and log target before doing things that can log
  LogOpts selfLogOpts;
  logOpts = &selfLogOpts;
  selfLogOpts.prog = "run-test";
  selfLogOpts.target = stderr;
  if (!getRaw(selfLogOpts.raw)) {
    return EXIT_FAILURE;
  };

  const fs::path testDir{argv[1]};
  const fs::path outDir{argv[2]};
  const std::string config{(testDir / "Config.toml").string()};
  const std::array<const char *, 4> cmd{"67", config.data(), "-o", argv[2]};

  std::string ref;
  if (!(readToStr(testDir / "RefCli.txt", ref))) [[unlikely]] {
    return EXIT_FAILURE;
  }
  std::string out;
  LogOpts importizerLogOpts{true, "importizer", &out};
  logOpts = &importizerLogOpts;
  const int rtn{entry(cmd.size(), cmd.data())};
  logOpts = &selfLogOpts;

  const int refRtn{ref.contains("importizer: error: ") ? EXIT_FAILURE
                                                       : EXIT_SUCCESS};
  bool errored{};
  if ((errored |= refRtn != rtn)) {
    err("Mismatched return code: expected {}, got {}.", refRtn, rtn);
  }

  if ((errored |= out != ref)) {
    err("Mismatched CLI output, got");

    // Don't format importizer's output
    fmt::print(stderr, "{}", out);
  }

  const fs::path refDir{testDir / "ref"};
  errored |= fs::exists(refDir) && !cmpDir(outDir, refDir);

  return errored ? EXIT_FAILURE : EXIT_SUCCESS;
}
