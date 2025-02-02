# Introduction
Do you need help converting your header-based codebase to C++20 modules?
importizer can help

What can it do?
- Convert `#include` statements to `import` statement, as well as creating the GMF.
- Takes you on the way to modularizing your codebase by reducing lots of trivial work
- **The only thing left to do is manually choosing what to export**

importizer supports two modularization scheme:
- **Complete modularization**: You want to ditch header-based code altogether and embrace C++ modules. This is the default.
- **Transtitional modularization**: You want to have both a header-based interface and a module-based one for backward compatibility, facilitating a gradual switch. You can opt in by specifing `[Transitional]` in the setting file or `transitional` on the command line.

# Example
- See [Example.md](Example.md#output-example)

# Getting started
## Prebuilt executable
- Choose your OS
- Download from the continuous tag. These are **debug** versions with sanitizers and no optimization.
- Download from other release tags. These are **release** versions with optimizations.

## Building from source
- Run:
```bash
git clone --depth 1 https://github.com/msqr1/importizer && cd importizer &&
mkdir build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release &&
cmake --build . --config Release -j $(cmake -P ../nproc.cmake)
```
- The generated binary is called `importizer` in the current working directory

## Usage
- Specify command line arguments or
- Create a `importizer.toml` in the directory of the executable (or somewhere else and specify `-c`), add some settings.
- Run the program
- The output will be a list of file path, relative to `outDir` that need to go through manual exporting
- For default mode:
  - Add `export`/`export{`/`}` around exported entities
  - Compile using modules right away
- For transitional mode:
  - Add values of `mi_exportKeyword`/`mi_exportBlockBegin`/`mi_exportBlockEnd` around exported entities
  - You can keep the same compilation command to do a header-based compilation
  - To compile using modules, add `-D[value of mi_control]` when compiling every file, the compilation command will have to change.
  - See [an example](Example.md#transitional-compilation-example)

## Testing
- Add `-DTESTS=1` when configuring cmake
- Build, then `cd [build root]/test`
- Run `ctest`
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
                     |

- Behavior of include path searching (similar concept to specifying `-I`):

| Type          | Action                                                                   |
|---------------|--------------------------------------------------------------------------|
| Quoted        | 1. Relative to directory of the current file<br>2. Same as angle-bracket |
| Angle bracket | 1. Relative to each of `includePaths`                                    |

# Macros
- Modules, unlike headers, were explicitly designed to be encapsulated from macros and so they wouldn't leak them. If a macro is defined in a header, and the header get modularized, it will only exist in that module. Some ways to remedy:
  - Add the macro definition on the command line when compiling for the files that needed the macro (using `-D...`).
  - Refactor the macro definition into a separate header, `#include` that where the macro is needed, and add the new header to `ignoredHdrs`.
  - Add the macro-containing headers to the `ignoredHdrs` (their paired sources, if available will be treated as if they have a `main()`)

# Settings
- CLI settings will override config file settings
- Paths are relative to the current working directory for CLI settings, and relative to the config file for TOML settings.
- General flags/settings:

| CLI flag name               | TOML setting name  | Description                                                                                                                         | Value type   | Default value |
|-----------------------------|--------------------|-------------------------------------------------------------------------------------------------------------------------------------|--------------|---------------|
| -c, --config                | N/A                | Path to TOML configuration file (`.toml`), default to `importizer.toml`                                                             | String       | N/A           |
| -h, --help                  | N/A                | Print help and exit                                                                                                                 | N/A          | N/A           |
| -v, --version               | N/A                | Print version and exit                                                                                                              | N/A          | N/A           |
| -s, --std-include-to-import | stdIncludeToImport | Convert standard includes to `import std` or `import std.compat`                                                                    | Boolean      | `false`       |
| -l, --log-current-file      | logCurrentFile     | Print the current file being processed                                                                                              | Boolean      | `false`       |
| --include-guard-pat         | includeGuardPat    | Regex to match include guards that must match the entire guard. #pragma once is processed by default.                               | String       | `[^\s]+_H`    |
| -i, --in-dir                | inDir              | Input directory (required on the CLI or in the TOML file)                                                                           | String       | N/A           |
| -o, --out-dir               | outDir             | Output directory (required on the CLI or in the TOML file)                                                                          | String       | N/A           |
| --hdr-ext                   | hdrExt             | Header file extension                                                                                                               | String       | `.hpp`        |
| --src-ext                   | srcExt             | Source (also module implementation unit) file extension                                                                             | String       | `.cpp`        |
| --module-interface-ext      | moduleInterfaceExt | Module interface unit file extension                                                                                                | String       | `.cppm`       |
| --include-paths             | includePaths       | Include paths searched when converting include to import                                                                            | String array | `[]`          |
| --ignored-hdrs              | ignoredHdrs        | Paths relative to `inDir` of header files to ignore. Their paired sources, if available, will be treated as if they have a `main()` | String array | `[]`          |

- Transitional flags/settings (must be specified after `transitional` on the CLI or under `[Transitional]` in the setting file):

| CLI flag name           | TOML setting name   | Description                                                                                                                                                | Value type | Default value  |
|-------------------------|---------------------|------------------------------------------------------------------------------------------------------------------------------------------------------------|------------|----------------|
| -b, --back-compat-hdrs  | backCompatHdrs      | Generate headers that include the module file to preserve #include for users. Note that in the codebase itself the module file is still included directly. | Boolean    | `false`        |
| --mi-control            | mi_control          | Header-module switching macro identifier                                                                                                                   | String     | `CPP_MODULES`  |
| --mi-export-keyword     | mi_exportKeyword    | Exported symbol macro identifier                                                                                                                           | String     | `EXPORT`       |
| --mi-export-block-begin | mi_exportBlockBegin | Export block begin macro identifier                                                                                                                        | String     | `BEGIN_EXPORT` |
| --mi-export-block-end   | mi_exportBlockEnd   | Export block end macro identifier                                                                                                                          | String     | `END_EXPORT`   |
| --export-macros-path    | exportMacrosPath    | Export macros file path relative to outDir                                                                                                                 | String     | `Export.hpp`   |

# Some code style rule
- Max column width: 90
- Put comment to denote what type after case label for variant switch
- Keep settings' order in the README, the option struct, and their value-checking order the same.
- To determine order for new settings, optimize for the option struct size