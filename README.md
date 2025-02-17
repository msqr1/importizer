# Table of contents
- [Introduction](#introduction)
- [Getting started](#getting-started)
  - [Prebuilt executable](#prebuilt-executable)
  - [Building from source](#building-from-source)
  - [Usage](#usage)
  - [Output example](#output-example)
  - [Testing](#testing)
- [Behavior](#behavior)
- [Settings](#settings)
- [Modules' side effects](#modules-side-effects)
  - [Transitive includes](#transitive-includes)
  - [Non-local macros](#non-local-macros)
- [Some code style rule](#some-code-style-rule)

# Introduction
Do you need help converting your header-based codebase to C++20 modules?
importizer can help.

What can it do?
- Convert `#include` statements to `import` statement, as well as creating the GMF.
- Takes you on the way to modularizing your codebase by reducing lots of trivial work.
- **The only thing left to do is manually choosing what to export**.

importizer supports two modularization scheme:
- **Complete modularization**: You want to ditch header-based code altogether and embrace C++ modules. This is the default.
- **Transtitional modularization**: You want to have both a header-based interface and a module-based one for backward compatibility, facilitating a gradual switch. You can opt in by specifing `[Transitional]` in the setting file or `transitional` on the command line.

Requirements:
- Code and options are valid UTF-8.
- Code is valid C++.

This project uses [semantic versioning](https://semver.org).

# Getting started
## Prebuilt executable
- Choose your OS.
- Download from the continuous tag. These are **debug** versions with sanitizers and no optimization.
- Download from other release tags. These are **release** versions with optimizations.

## Building from source
- Run:
```bash
git clone --depth 1 https://github.com/msqr1/importizer && cd importizer &&
mkdir build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release &&
cmake --build . --config Release -j $(cmake -P ../nproc.cmake)
```
- The generated binary is called `importizer` in the current working directory.

## Usage
- [Follow along](Examples/FollowAlong.md) (incomplete).

### Before running
1. **Handle non-local macros as well as transitive includes** because they are incompatible with modules (see [Modules' side effects](#modules-side-effects)).
2. **Acquire the correct settings for your project**, and add them into `importizer.toml` in the directory of the executable (or somewhere else and specify `-c`).
3. The TOML file is the recommended way, though you can specify command-line arguments to quickly test something out.

### Run the program

### After running
1. The output will be a list of file path, relative to `outDir` that need to go through manual exporting. You can redirect into a text file for easy viewing.
2. **Perform preamble sanity checks for all files**. Don't refactor or export yet, you may need to rerun the program. Common error is forgetting a setting. If the program fails to generate valid code whatsoever, please file an issue.
3. Do depending on each mode:
    - Default:
      1. **Export**: Add `export` or `export {` and `}` around exported entities in the files outputted by the program.
      2. **Try compile the modularized project**.
    - Transitional (see a [mini demo](Examples/MiniTransitional.md)):
      1. **Try to compile using header**, you shouldn't have to change anything for this to work. If it doesn't, then importizer has an issue, please report it.
      2. **Export**: Add values of `mi_exportKeyword` or `mi_exportBlockBegin` and `mi_exportBlockEnd` around exported entities in the files outputted by the program.
      3. **Try compile using modules**, add `-D[value of mi_control]` when compiling every file to enable module mode.
4. **Refactor** (this list will get shorter over time):
    - **Refactor directives**: Importizer doesn't have enough context for the project it's modularizing, so it recreate the exact directive structure to ensure outside includes only happens in the original conditions.
    - **Refactor directives 2**: Since includes are moved up, they may leave unecessary directives at their original location.
    - **Reposition the SOF (start-of-file) (usually license) comments** because importizer always place the preamble on the very top of the file.

## [Output example](Examples/Output.md)

## Testing
- Add `-DTESTS=1` when configuring cmake.
- Build, then `cd [build root]/test`.
- Run `ctest`.
- If possible, file an issue for test(s) that failed.

# Behavior
- A file pair is defined as one header and one with the same basename (filename without extension) in the same directory. For example, `input/directory/file.cpp` and `input/directory/file.hpp`. The paired source must #include and defines everything declared in the paired header.
- importizer's behavior is undefined for a header-source pair and the source has a main function. Very rare in real code.
- Action by file type:

| File type | Paired             | Has `main()`       | Conversion                 | Must do manual export |
|-----------|--------------------|--------------------|----------------------------|-----------------------|
| Header    |                    | N/A                | Module interface unit      | :heavy_check_mark:    |
| Header    | :heavy_check_mark: | N/A                | Module interface unit      | :heavy_check_mark:    |
| Source    |                    |                    | Module interface unit      | :heavy_check_mark:    |
| Source    | :heavy_check_mark: |                    | Module implementation unit |                       |
| Source    |                    | :heavy_check_mark: | Only include to import     |                       |

- Behavior of include path searching (similar concept to specifying `-I`):

| Type          | Action                                                                   |
|---------------|--------------------------------------------------------------------------|
| Quoted        | 1. Relative to directory of the current file<br>2. Same as angle-bracket |
| Angle bracket | 1. Relative to each of `includePaths`                                    |

# Settings
- CLI settings will override config file settings.
- Paths are relative to the current working directory for CLI settings, and relative to the config file for TOML settings.
- General flags/settings:

| CLI flag name               | TOML setting name  | Description                                                                                                                                                                 | Value type   | Default value |
|-----------------------------|--------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------|--------------|---------------|
| -c, --config                | N/A                | Path to TOML configuration file (`.toml`), default to `importizer.toml`.                                                                                                    | String       | N/A           |
| -h, --help                  | N/A                | Print help and exit.                                                                                                                                                        | N/A          | N/A           |
| -v, --version               | N/A                | Print version and exit.                                                                                                                                                     | N/A          | N/A           |
| -s, --std-include-to-import | stdIncludeToImport | Convert standard includes to `import std` or `import std.compat`.                                                                                                           | Boolean      | `false`       |
| -l, --log-current-file      | logCurrentFile     | Print the current file being processed.                                                                                                                                     | Boolean      | `false`       |
| -p, --pragma-once           | pragmaOnce         | Declare that you use `#pragma once` so importizer handles them.                                                                                                             | Boolean      | `false`       |
| -S, --SOF-comments          | SOFComments        | Declare that your files may start with comments (usually to specify a license) so importizer handles them. Note that it scans for the largest continuous SOF comment chain. | Boolean      | `false`       |
| --include-guard             | includeGuard       | Declare that you use include guards so the tool handles them. You will provide a regex to match match the entire guard. Example: `[^\s]+_H`.                                | String       | N/A           |
| -i, --in-dir                | inDir              | Input directory (required on the CLI or in the TOML file).                                                                                                                  | String       | N/A           |
| -o, --out-dir               | outDir             | Output directory (required on the CLI or in the TOML file).                                                                                                                 | String       | N/A           |
| --hdr-ext                   | hdrExt             | Header file extension.                                                                                                                                                      | String       | `.hpp`        |
| --src-ext                   | srcExt             | Source (also module implementation unit) file extension.                                                                                                                    | String       | `.cpp`        |
| --module-interface-ext      | moduleInterfaceExt | Module interface unit file extension.                                                                                                                                       | String       | `.cppm`       |
| --include-paths             | includePaths       | Include paths searched when converting include to import.                                                                                                                   | String array | `[]`          |
| --ignored-hdrs              | ignoredHdrs        | Paths relative to `inDir` of header files to ignore. Their paired sources, if available, will be treated as if they have a `main()`.                                        | String array | `[]`          |
| --umbrella-hdrs             | umbrellaHdrs       | Paths relative to `inDir` of modularized headers, but their `import` are turned into `export import`.                                                                       | String array | `[]`          |

- Transitional flags/settings (must be specified after `transitional` on the CLI or under `[Transitional]` in the setting file):

| CLI flag name           | TOML setting name   |Description                                                                                                                                                 | Value type | Default value  |
|-------------------------|---------------------|------------------------------------------------------------------------------------------------------------------------------------------------------------|------------|----------------|
| -b, --back-compat-hdrs  | backCompatHdrs      | Generate headers that include the module file to preserve #include for users. Note that in the codebase itself the module file is still included directly. | Boolean    | `false`        |
| --mi-control            | mi_control          | Header-module switching macro identifier.                                                                                                                  | String     | `CPP_MODULES`  |
| --mi-export-keyword     | mi_exportKeyword    | Exported symbol macro identifier.                                                                                                                          | String     | `EXPORT`       |
| --mi-export-block-begin | mi_exportBlockBegin | Export block begin macro identifier.                                                                                                                       | String     | `BEGIN_EXPORT` |
| --mi-export-block-end   | mi_exportBlockEnd   | Export block end macro identifier.                                                                                                                         | String     | `END_EXPORT`   |
| --export-macros-path    | exportMacrosPath    | Export macros file path relative to outDir.                                                                                                                | String     | `Export.hpp`   |

# Modules' side effects

## Transitive includes
- Modules are completely separate from each other. Transitive includes is not propogated across imported modules. Way(s) to remedy:
  - Include everything you use before modularization, so the tool would keep it automatically. You could use clangd with strict missing and unused include to help you.

## Non-local macros
- Modules, unlike headers, were explicitly designed to be encapsulated from macros and so they wouldn't leak them. Local macros only used in the file that defined them are OK. If a macro is defined in a header, and the header get modularized, it will only exist in that module. Way(s) to remedy:
  - Add the macro definition on the command line when compiling for the files that needed the macro (using `-D...`).
  - Refactor the macro definition into a separate header, `#include` that where the macro is needed, and add the new header to `ignoredHdrs`.
  - Add the macro-containing headers to the `ignoredHdrs` (recommended when the header's sole purpose is to provide macros).

# Some rules
- Max column width: 90, one or two past that is fine, but should not be abused.
- Put comment to denote what type after case label for variant switch.
- Keep settings' order in the README, the option struct, and their value-checking order the same.
- To determine order for new settings, optimize for the option struct size and follow chronological order.
- Off-by-default rule: All settings must have a special "false" default value (eg. empty for arrays or `false` for boolean), or else must be converted to an std::optional if they solve a problem that may not be needed by users. Current ones are:
  - pragmaOnce
  - SOFComments
  - includeGuard
  - includePaths
  - ignoredHdrs
  - umbrellaHdrs