#include <filesystem>
namespace fs = std::filesystem;

bool cmpDir(const fs::path &dir, const fs::path &ref) noexcept;
