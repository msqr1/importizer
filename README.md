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

- Select your operating system:
  - Windows (10+ only)
  - MacOS (14+ only)
  - Linux (glibc 2.35+ only)
- Download the **debug** versions (with sanitizers and no optimizations) from the [continuous tag](https://github.com/msqr1/importizer/releases/tag/continuous).
- For production use, download the **release** versions (with optimizations) from the other releases.

## Building from Source

```bash
git clone --depth 1 https://github.com/msqr1/importizer &&
cd importizer &&
mkdir build &&
cd build &&
cmake .. -DCMAKE_BUILD_TYPE=Release &&
cmake --build . --config Release -j $(cmake -P ../nproc.cmake)
```

- The executable `importizer` will be located in `build`

# Developing

## Executable-dependencies information

|                 | Linux                 | MacOS                    | Windows             |
| --------------- | --------------------- | ------------------------ | ------------------- |
| LibTooling      | Dynamic<sup>1</sup>   | Dynamic<sup>1</sup>      | Static              |
| LLVM            | Dynamic<sup>1</sup>   | Dynamic<sup>1</sup>      | Static              |
| LLVM version    | 22.1.6                | 22.1.4                   | 22.1.4              |
| LibC            | Dynamic<sup>4</sup>   | Dynamic<sup>2</sup>      | Dynamic<sup>3</sup> |
| LibC++          | Static                | Dynamic<sup>2</sup>      | Static              |
| Other 3rd-party | Static                | Static                   | Static              |
| Distributed as  | tar(Executable + .so) | tar(Executable + .dylib) | Executable          |

1. Package manager doesn't offer static version, pending self-built LLVM
2. Dynamic with stable ABI guaranteed by macOS (libSystem), we can treat as if static
3. Dynamic with stable ABI guaranteed by Windows (UCRT), we can treat as if static
4. Dynamic glibc, pending self-built static LLVM-libc

## Testing

- Add `-DBUILD_TESTING=1` when configuring CMake.
- Build, then `cd [build root]/test`.
- Run `ctest`.
- If possible, file an issue for test(s) that failed.

## Contribution Rules

- Use camelCase for C++ variables and functions
- Use PascalCase for C++ classes, types, and all filenames (except for executables use kebab-case).
- Use ALL_CAPS_SNAKE_CASE for CMake external variables
- Follow LLVM coding style
- Always use strict IWYU.
- Keep the maximum line width around 90 columns.
- Add comments to indicate types after each case label in variant switches.
- Maintain consistent ordering of options in the README, `struct Opts`, and value-checking logic.
