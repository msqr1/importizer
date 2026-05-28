#include <filesystem>
namespace fs = std::filesystem;

[[nodiscard]] bool cmpDir(const fs::path &dir, const fs::path &ref) noexcept;
