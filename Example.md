# Example
## Before
- FileOp.hpp:
```cpp
#pragma once
#include <string>
#include <filesystem>

void readFromPath(const std::filesystem::path& path, std::string& str);
```
- FileOp.cpp:
```cpp
#include "FileOp.hpp"
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;
void readFromPath(const fs::path& path, std::string& str) {
  std::ifstream ifs{path, std::fstream::binary};
  ifs.exceptions(std::fstream::failbit | std::fstream::badbit);
  ifs.read(str.data(), fs::file_size(path));
}
```

## After (default):
- FileOp.cppm:
```cpp
module;
#include <string>
#include <filesystem>
export module FileOp;


void readFromPath(const std::filesystem::path& path, std::string& str);
```
- FileOp.cpp:
```cpp
module;
#include <fstream>
#include <filesystem>
module FileOp;


namespace fs = std::filesystem;
void readFromPath(const fs::path& path, std::string& str) {
  std::ifstream ifs{path, std::fstream::binary};
  ifs.exceptions(std::fstream::failbit | std::fstream::badbit);
  ifs.read(str.data(), fs::file_size(path));
}
```

## After (transitional):
- FileOp.cppm:
```cpp
#pragma once
#include "Export.hpp"
#ifdef CPP_MODULES
module;
#include <string>
#include <filesystem>
export module FileOp;
#else
#include <string>
#include <filesystem>
#endif


void readFromPath(const std::filesystem::path& path, std::string& str);
```
- FileOp.cpp:
```cpp
#include "Export.hpp"
#ifdef CPP_MODULES
module;
#include <fstream>
#include <filesystem>
#include <string>
module FileOp;
#else
#include "FileOp.cppm"
#include <fstream>
#include <filesystem>
#include <string>
#endif


namespace fs = std::filesystem;
void readFromPath(const fs::path& path, std::string& str) {
  std::ifstream ifs{path, std::fstream::binary};
  ifs.exceptions(std::fstream::failbit | std::fstream::badbit);
  ifs.read(str.data(), fs::file_size(path));
}
```
- Export.hpp:
```cpp
#ifdef CPP_MODULES
#define EXPORT export
#define BEGIN_EXPORT export {
#define END_EXPORT }
#else
#define EXPORT
#define BEGIN_EXPORT
#define END_EXPORT
#endif
```