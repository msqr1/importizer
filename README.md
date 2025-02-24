# Table of Contents
- [Introduction](#introduction)
- [Getting Started](#getting-started)
  - [Prebuilt Executable](#prebuilt-executable)
  - [Building from Source](#building-from-source)
  - [Usage](#usage)
  - [Output Example](#output-example)
  - [Testing](#testing)
- [Behavior](#behavior)
- [Options](#options)
- [Modules' Side Effects](#modules-side-effects)
  - [Transitive Includes](#transitive-includes)
  - [Non-Local Macros](#non-local-macros)
- [Development Guidelines](#development-guidelines)

# Introduction
Are you looking to convert your header-based C++ codebase to C++20 modules? **importizer** is here to help.

## What importizer Does:
- Converts `#include` directives to `import` statements and generates the Global Module Fragment (GMF).
- Automates the modularization process by handling much of the repetitive work.
- **Note:** After running importizer, you still need to manually decide which entities to export.

## Modularization Modes:
- **Complete Mode:** Fully transition away from header-based code to C++ modules (default).
- **Transitional Mode:** Maintain both a header-based interface and a module-based one for backward compatibility. Enable this mode by specifying `[transitional]` in the settings file or by using `transitional` as the first command-line argument.

## Requirements:
- All source code and configuration options must be valid UTF-8.
- The code must adhere to valid C++ standards.

This project follows [semantic versioning](https://semver.org).

# Getting Started

## Prebuilt Executable
- Select your operating system.
- Download the **debug** versions (with sanitizers and no optimizations) from the continuous tag.
- For production use, download the **release** versions (with optimizations) from the other release tags.

## Building from Source
Clone the repository and build with the following commands:
```bash
git clone --depth 1 https://github.com/msqr1/importizer && cd importizer &&
mkdir build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release &&
cmake --build . --config Release -j $(cmake -P ../nproc.cmake)
```
- The resulting binary, named `importizer`, will be located in your current working directory.

## Usage Walkthrough
- [Follow along](Examples/README.md#follow-along-folders) 

### Step 1: Preparation
1. **(Optional) Relocate sources and headers:**: Move your source and header files to a different folder. This new folder becomes your input directory, while the original folder remains your output directory. This setup allows you to re-run importizer repeatedly and test builds without affecting other files (like `CMakeLists.txt`).
2. **Handle non-local macros, transitive includes, and forward declarations** because they don't work well with modules (see [Modules' side effects](#modules-side-effects)).
3. **Acquire the correct options**: Populate the importizer.toml file (or use the -c flag) with the correct settings for your project. Using a TOML configuration file is recommended, although command-line arguments can be used for quick tests

### Step 2: Run the Program
- When you run importizer, the command-line output will list file paths (relative to outDir) that need manual export modifications. You can redirect this output to a text file for easier review.

### Step 3: Post-Execution Actions
1. **Perform preamble checks**. Review all generated preambles in files for correctness before making any modifications. If the program fails to generate valid code whatsoever, please file an issue.
2. If you are in default mode:
    1. **Add export**: For each outputted file, manually insert `export` or wrap exported entities with `export { ... }`
    2. **Test-compile the modularized project**.
3. If you are in transitional mode:
    1. **Test-compile using header**. You shouldn't have to change anything for this to work. If it doesn't, then importizer has an issue, please report it.
    2. **Add export**: For each file outputted, add the values of `mi_exportKeyword`, `mi_exportBlockBegin` and `mi_exportBlockEnd` around exported entities.
    3. **Test-compile using modules**. Add `-D[value of mi_control]` when compiling all files to enable module mode.
4. **Clean-up Procedures** (list will shorten over time):
    - **Clean up directives**: Importizer rebuilds the original directive structure to ensure that external includes only happens in their initial conditions. The minimizer attempts to shorten this structure, though it may leave some redundant directives without enough context.
    - **Clean up directives 2**: Since includes are relocated, remove any unnecessary directives left in the original file locations.

## [Output example](Examples/Output.md)

# File pairing and conversion rules
A file pair consists of a header and a source file sharing the same basename within the same directory (for example, input/directory/file.cpp paired with input/directory/file.hpp). The source file must include and define everything declared in its paired header.
Note: Importizer’s behavior is undefined if a paired source file contains a main() function.
- Actions by file type:

| File type | Paired file?       | Contains `main()`? | Conversion type            | Exporting required |
|-----------|--------------------|--------------------|----------------------------|--------------------|
| Header    |                    | N/A                | Module interface unit      | :heavy_check_mark: |
| Header    | :heavy_check_mark: | N/A                | Module interface unit      | :heavy_check_mark: |
| Source    |                    |                    | Module interface unit      | :heavy_check_mark: |
| Source    | :heavy_check_mark: |                    | Module implementation unit |                    |
| Source    |                    | :heavy_check_mark: | Only include to import     |                    |

- Behavior of include path resolution (similar concept to specifying `-I`):

| Type          | How it is resolved                                                                   |
|---------------|--------------------------------------------------------------------------------------|
| Quoted        | 1. Relative to the directory of the current file<br>2. If not, same as angle-bracket |
| Angle bracket | 1. Searched relative to each directory specified in `includePaths`                   |

# Options
Customize importizer’s behavior via the command line or a TOML configuration file (CLI options override TOML settings). Note that CLI paths are relative to the current working directory, whereas TOML paths are relative to the configuration file.

## General options:

| CLI flag name               | TOML setting name  | Description                                                                                                                                                                 | Value type   | Default value |
|-----------------------------|--------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------|--------------|---------------|
| -h, --help                  | N/A                | Print help and exit.                                                                                                                                                        | N/A          | N/A           |
| -v, --version               | N/A                | Print version and exit.                                                                                                                                                     | N/A          | N/A           |
| -s, --std-include-to-import | stdIncludeToImport | Convert standard includes to `import std` or `import std.compat`.                                                                                                           | Boolean      | `false`       |
| -p, --pragma-once           | pragmaOnce         | Declare that you use `#pragma once` so importizer handles them.                                                                                                             | Boolean      | `false`       |
| -S, --SOF-comments          | SOFComments        | Declare that your files may start with comments (usually to specify a license) so importizer handles them. Note that it scans for the largest continuous SOF comment chain. | Boolean      | `false`       |
| --include-guard             | includeGuard       | Declare that you use include guards so the tool handles them. You will provide a regex to match match the entire guard, for example: `[^\s]+_HPP`.                          | String       | N/A           |
| -c, --config                | N/A                | Path to TOML configuration file (`.toml`), default to `importizer.toml`.                                                                                                    | String       | N/A           |
| -i, --in-dir                | inDir              | Input directory (required on the CLI or in the TOML file).                                                                                                                  | String       | N/A           |
| -o, --out-dir               | outDir             | Output directory (required on the CLI or in the TOML file).                                                                                                                 | String       | N/A           |
| --hdr-ext                   | hdrExt             | Header file extension.                                                                                                                                                      | String       | `.hpp`        |
| --src-ext                   | srcExt             | Source (also module implementation unit) file extension.                                                                                                                    | String       | `.cpp`        |
| --module-interface-ext      | moduleInterfaceExt | Module interface unit file extension.                                                                                                                                       | String       | `.ixx`        |
| --include-paths             | includePaths       | Include paths searched when converting include to import.                                                                                                                   | String array | `[]`          |
| --ignored-hdrs              | ignoredHdrs        | Paths relative to `inDir` of header files to ignore. Their paired sources, if available, will be treated as if they have a `main()`.                                        | String array | `[]`          |
| --umbrella-hdrs             | umbrellaHdrs       | Paths relative to `inDir` of modularized headers, but their `import` are turned into `export import`.                                                                       | String array | `[]`          |

## Transitional options
Activate transitional mode by specifying transitional as the first CLI argument or under [transitional] in the settings file.

| CLI flag name           | TOML setting name   | Description                                                                                                                                                | Value type | Default value  |
|-------------------------|---------------------|------------------------------------------------------------------------------------------------------------------------------------------------------------|------------|----------------|
| -b, --back-compat-hdrs  | backCompatHdrs      | Generate headers that include the module file to preserve #include for users. Note that in the project itself the module file is still included directly.  | Boolean    | `false`        |
| --mi-control            | mi_control          | Header-module switching macro identifier.                                                                                                                  | String     | `CPP_MODULES`  |
| --mi-export-keyword     | mi_exportKeyword    | Exported symbol macro identifier.                                                                                                                          | String     | `EXPORT`       |
| --mi-export-block-begin | mi_exportBlockBegin | Export block begin macro identifier.                                                                                                                       | String     | `BEGIN_EXPORT` |
| --mi-export-block-end   | mi_exportBlockEnd   | Export block end macro identifier.                                                                                                                         | String     | `END_EXPORT`   |
| --export-macros-path    | exportMacrosPath    | File path relative to outDir to store the export macros above.                                                                                             | String     | `Export.hpp`   |

# Modules' side effects

## Non-local macros
- C++ modules are designed to encapsulate macros, meaning that a macro defined in a header will become local to its corresponding module. To ensure macros remain globally available:
  - Define the macro via compiler command-line (using `-D...`).
  - Refactor the macro into a separate header, include it where necessary, and add it to `ignoredHdrs`.
  - Add the macro-containing headers directly to `ignoredHdrs` (recommended when the header's sole purpose is to provide macros).

## Transitive includes
- Modules do not propagate transitive includes. To avoid missing dependencies:
  - Include what you use (IWYU) before modularization, so the tool would keep it automatically. Use tools like clangd with strict IWYU to help you.
 
## Forward declarations
- When converting a header to a module, any forward declarations become part of that module. This can lead to conflicts if the full declaration exists in another module.
  - If forward declarations are used to break cyclic dependencies, refactor the shared entity into its own file and include it in both modules.
  - (Only way for external forward declarations) Include the full declaration by #include-ing the corresponding file and removing the forward declaration. Note that this may lengthen header build in transitional mode.
  - In transitional mode, if you ABSOLUTELY need to save on header compile time, you might adjust the modularized preamble as follow:
    ```cpp
    #ifdef CPP_MODULES // Replace with your mi_control value
    // Import modules that export the forward-declared entity
    #else
    // Place forward declarations here
    #endif
    ```

# Developing

## Testing
- Add `-DTESTS=1` when configuring CMake.
- Build, then `cd [build root]/test`.
- Run `ctest`.
- If possible, file an issue for test(s) that failed.

## Precompiled headers
To significantly reduce compile times for incremental build (up to ~70%) (some workarounds are required, so I prepared a setting.):
- Add `-DPCH=1` when configuring CMake.


## Contribution rules
- Use camelCase for variables and functions; use PascalCase for classes, types, and filenames.
- Follow the coding style of the surrounding code.
- Always use strict IWYU.
- Keep the maximum line width around 90 columns (a slight exceedance of 1 or 2 is acceptable but should be minimized).
- Add comments to indicate types after each case label in variant switches.
- Maintain consistent ordering of options in the README, option structures, and value-checking logic.
- To determine order for new options, optimize for the option struct size and follow chronological order.
- All options should have a default “false” or empty state unless absolutely required. Options with non-false defaults should be managed as std::optional if they solve non-critical issues. Current options include:
  - `pragmaOnce`
  - `SOFComments`
  - `includeGuard`
  - `includePaths`
  - `ignoredHdrs`
  - `umbrellaHdrs`
