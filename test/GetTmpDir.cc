#include <filesystem>
#include <fmt/base.h>
int main() {
  fmt::print((std::filesystem::temp_directory_path() / "include2import").native());
}