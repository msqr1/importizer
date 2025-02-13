# Output example
All default, no setting

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

## After (default, ran with `importizer`):
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

## After (transitional, ran with `importizer transitional`):
- FileOp.cppm:
```cpp
#pragma once
#ifdef CPP_MODULES
module;
#endif
#include "Export.hpp"
#include <string>
#include <filesystem>
#ifdef CPP_MODULES
export module FileOp;
#else
#endif


void readFromPath(const std::filesystem::path& path, std::string& str);
```
- FileOp.cpp:
```cpp
#ifdef CPP_MODULES
module;
#endif
#include "Export.hpp"
#include <fstream>
#include <filesystem>
#ifdef CPP_MODULES
module FileOp;
#else
#include "FileOp.cppm"
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