# Introduction

Are you looking to convert your header-based C++ codebase to C++20 modules, but afraid that modularizing is too cumbersome? **importizer** is here to help.

**Note:** Importizer is currently going through a complete rewrite using Clang LibTooling to replace Regex (pre-2.0.0). Much pre-2.0.0 things are gone, but releases and git history are still preserved.

## Wait, Briefly, Why C++ Modules?

- **Improved compilation time:** Modules don't need to be recompiled every time it is imported, unlike headers.
- **Improved encapsulation:** Modules allow you to choose what is exposed to users (exported), no need to hide stuff with a `detail` namespace. Includes and macros from a module will only stay in that module.

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
cmake -B build -DCMAKE_BUILD_TYPE=Release &&
cmake --build build --config Release -j $(cmake -P ../nproc.cmake)
```

- The executable `importizer` will be located in `build`

# Developing

## Executable-dependencies information

|                 | Linux                 | MacOS                    | Windows                |
| --------------- | --------------------- | ------------------------ | ---------------------- |
| LibTooling      | Dynamic<sup>1</sup>   | Dynamic<sup>1</sup>      | Dynamic<sup>1</sup>    |
| LLVM            | Dynamic<sup>1</sup>   | Dynamic<sup>1</sup>      | Dynamic<sup>1</sup>    |
| LibC            | Dynamic<sup>4</sup>   | Dynamic<sup>2</sup>      | Dynamic<sup>3</sup>    |
| LibC++          | Static                | Dynamic<sup>2</sup>      | Static                 |
| Other 3rd-party | Static                | Static                   | Static                 |
| Distributed as  | tgz(Executable + .so) | tgz(Executable + .dylib) | zip(Executable + .dll) |

1. Package manager doesn't offer static version, pending self-built LLVM
2. Dynamic with stable ABI guaranteed by macOS (libSystem), we can treat as if static
3. Dynamic with stable ABI guaranteed by Windows (UCRT), we can treat as if static
4. Dynamic glibc, pending switch to musl

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
