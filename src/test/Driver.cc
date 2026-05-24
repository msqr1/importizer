#include "subprocess.h"
#include "utils/Control.hh"
#include "utils/FileOp.hh"
#include <array>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <string>
#include <string_view>

namespace fs = std::filesystem;

std::string_view prog() { return "test"; }

// test [importizer path] [testDir] [outDir]
void run(const int argc, const char **argv) {
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
    exitWithErr("importizer failed to start\n");
  }
  int exitCode;
  res = subprocess_join(&proc, &exitCode);
  if (res != 0) {
    exitWithErr("Unable to wait for importizer\n");
  }
  std::string out;
  readToStr(subprocess_stdout(&proc), out, "importizer stdout");
  std::string ref;
  readToStr(testDir / "RefCli.txt", ref);
}

int main(const int argc, const char **argv) {
  try {
    run(argc, argv);
  } catch (const int code) {
    return code;
  } catch (const std::exception &e) {
    err("{}\n", e.what());
    return EXIT_FAILURE;
  }
}
