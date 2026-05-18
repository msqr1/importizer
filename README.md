# Introduction

Are you looking to convert your header-based C++ codebase to C++20 modules, but afraid that modularizing is too cumbersome? **importizer** is here to help.

## Wait, Briefly, Why C++ Modules?

- **Improved compilation time:** Modules don't need to be recompiled every time it is imported, unlike headers.
- **Improved encapsulation:** Modules allow you to choose what is exposed to users (exported), no need to hide stuff with a `detail` namespace. Includes and macros from a module will only stay in that module.

## What importizer Does Automatically

- Name modules according to its relativity to the modularized directory.
- Converts `#include` directives to `import` statements.
- Handle include guards and pragma once.
- Handle start-of-file comments (usually licence).

## What importizer Does NOT Do Automatically

- Export entities for you, you will have to manually do this after running.

## Modularization Modes

- **Complete Mode:** Fully transition away from header-based code to C++ modules (default).
- **Transitional Mode:** Provide both header-based interface and a module-based one for backward compatibility without duplicating code. Enable this mode by specifying `[transitional]` in the settings file or by using `transitional` as the first command-line argument.

## Requirements

- All source code and configuration options must be valid UTF-8.
- The code must be valid C++.

This project follows [semantic versioning](https://semver.org).

# Getting Started

## Prebuilt Executable

- Select your operating system.
- Download the **debug** versions (with sanitizers and no optimizations) from the continuous tag.
- For production use, download the **release** versions (with optimizations) from the other release tags.

## Building from Source

```bash
git clone --depth 1 https://github.com/msqr1/importizer &&
cd importizer &&
mkdir build &&
cd build &&
cmake .. -DCMAKE_BUILD_TYPE=Release &&
cmake --build . --config Release -j $(cmake -P ../nproc.cmake)
```

- The resulting binary, named `importizer`, will be located in your current working directory.

# Developing

## Testing

- Add `-DTESTS=1` when configuring CMake.
- Build, then `cd [build root]/test`.
- Run `ctest`.
- If possible, file an issue for test(s) that failed.

## Contribution Rules

- Use camelCase for variables and functions; use PascalCase for classes, types, and filenames.
- Follow the coding style of the surrounding code.
- Always use strict IWYU.
- Keep the maximum line width around 90 columns (a slight exceedance of 1 or 2 is acceptable but should be minimized).
- Add comments to indicate types after each case label in variant switches.
- Maintain consistent ordering of options in the README, option structures, and value-checking logic.
- To determine order for new options, optimize for the option struct size and follow chronological order.
- All options should have a default “false” or empty state unless absolutely required. Options with non-false defaults should be managed as `std::optional` if they solve non-critical issues. Current options include:
  - `pragmaOnce`
  - `SOFComments`
  - `includeGuard`
  - `includePaths`
  - `ignoredHdrs`
  - `umbrellaHdrs`
