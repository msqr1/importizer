# Output example
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

# Transitional compilation example
## Say we have
- moduleTest.cppm:
```cpp
#ifdef CPP_MODULES
module;
#endif
#include "Export.hpp"
#ifdef CPP_MODULES
export module test;
#else
#endif
EXPORT int x();
```
- moduleTest.cpp:
```cpp
#ifdef CPP_MODULES
module;
#endif
#ifdef CPP_MODULES
module test;
#define A 1
#else
#define A 2
#endif
int x() {
  return A;
}
```
- moduleMain.cpp
```cpp
#ifdef CPP_MODULES
#include <iostream>
import test;
#else
#include <iostream>
#include "moduleTest.cppm"
#endif
int main() {
  std::cout << x();
}
```
- Export.hpp (same as above)

## Compiling in header mode (clang)
- Command should mostly be the same as before modularization:
```clang++ moduleMain.cc moduleTest.cc -std=c++20 -o main```
- `./main` output: `2`

## Compiling in module mode (clang)
- `-std=c++20` is required to activate module compilation
- Compile the module interface unit into a precompiled module (.pcm)
- Compile the module implementation unit and the .pcm into an object file
- Compile the main file and the object file into an executable
```bash
clang++ moduleTest.cppm -DCPP_MODULES -std=c++20 --precompile -o moduleTest.pcm &&
clang++ moduleTest.cc -DCPP_MODULES -std=c++20 -fmodule-file=test=moduleTest.pcm -c -o moduleTest.o &&
clang++ moduleMain.cc -DCPP_MODULES moduleTest.o -std=c++20 -fmodule-file=test=moduleTest.pcm moduleTest.pcm -o main
```
- `./main` output: `1`