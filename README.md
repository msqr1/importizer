# Introduction
Do you need help converting your header-based codebase to C++20 modules?
importizer can help.

What does it do?
- Convert ```#include``` statements to ```import``` statement, as well as creating the GMF
- Takes you on the way to modularizing your codebase.
- **The only thing left to do is manually choosing what to export.**

importizer supports two modularization scheme:
- Complete modularization: You want to ditch header-based code altogether and embrace C++ modules. This is the default.
- Transtitional modularization: You want to have both a header-based interface and a module-based one for backward compatibility, facilitating a gradual switch. You can opt in by specifing ```[Transitional]``` in the setting file or ```transitional``` on the command line.

# Getting started
## Prebuilt executable
- Choose your OS
- Download from the continuous tag. These are ***debug** versions with sanitizers and no optimization.
- Download from other release tags. These are **release** versions with optimizations.

## Building from source
- Run:
```
git clone --depth 1 https://github.com/msqr1/importizer && cd importizer &&
mkdir build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release &&
cmake --build . -j $(cmake -P ../nproc.cmake)
```
- The generated binary is called ```importizer``` in the current working directory

## Usage
- Specify command line arguments or
- Create a ```importizer.toml``` in the directory of the executable (or somewhere else and specify ```-c```), add some settings.
- Run the program
- The output will be a list of file path, relative to ```outDir``` that need to go through manual exporting
- For default mode, just add ```export``` (value of ```mi_exportKeyword``` for transitional mode) around the entities that you want to be exported.

# CLI
- Arguments will take precedence over those in the config file
- Paths are relative to the current working directory unless specified otherwise
- General flags:

| Name                        | Description                                                                                                                                 |
|-----------------------------|---------------------------------------------------------------------------------------------------------------------------------------------|
| -c, --config                | Path to TOML configuration file (```.toml```), default to ```importizer.toml```                                                             |
| -h, --help                  | Print help and exit                                                                                                                         |
| -v, --version               | Print version and exit                                                                                                                      |
| -s, --std-include-to-import | Convert standard includes to ```import std``` or ```import std.compat```                                                                    |
| -l, --log-current-file      | Print the current file being processed                                                                                                      |
| --include-guard-pat         | Regex to match include guards. #pragma once is processed by default                                                                         |
| -i, --in-dir                | Input directory (required if not specified in the config file)                                                                              |
| -o, --out-dir               | Output directory (required if not specified in the config file)                                                                             |
| --hdr-ext                   | Header file extension                                                                                                                       |
| --src-ext                   | Source (also module implementation unit) file extension                                                                                     |
| --module-interface-ext      | Module interface unit file extension                                                                                                        |
| --include-paths             | Include paths searched when converting include to import                                                                                    |
| --ignored-hdrs              | Paths relative to ```inDir``` of header files to ignore. Their paired sources, if available, will be treated as if they have a ```main()``` |

- Transitional modularization flag (must be specified after ```transitional``` on the CLI):

| Name                    | Description                                                                                                                                                | Value type | Default value      |
|-------------------------|------------------------------------------------------------------------------------------------------------------------------------------------------------|------------|--------------------|
| -b, --back-compat-hdrs  | Generate headers that include the module file to preserve #include for users. Note that in the codebase itself the module file is still included directly. | Boolean    | ```false```        |
| --mi-control            | Header-module switching macro identifier                                                                                                                   | string     | ```CPP_MODULES```  |
| --mi-export-keyword     | Exported symbol macro identifier                                                                                                                           | string     | ```EXPORT```       |
| --mi-export-block-begin | Export block begin macro identifier                                                                                                                        | string     | ```BEGIN_EXPORT``` |
| --mi-export-block-end   | Export block end macro identifier                                                                                                                          | string     | ```END_EXPORT```   |
| --export-macros-path    | Export macros file path relative to outDir                                                                                                                 | string     | ```Export.hpp```   |

# TOML setting file
- Paths are relative to the config file by default, unless otherwise specified
- Settings are optional, unless otherwise specified
- Value types and default values are in the section below
- General settings:

| Name               | Description                                    | Value type   | Default value  |
|--------------------|------------------------------------------------|--------------|----------------|
| stdIncludeToImport | As in CLI                                      | Boolean      | ```false```    |
| logCurrentFile     | As in CLI                                      | Boolean      | ```false```    |
| includeGuardPat    | As in CLI                                      | String       | ```[^\s]+_H``` |
| inDir              | As in CLI (required if unspecified on the CLI) | String       | N/A            |
| outDir             | As in CLI (required if unspecified on the CLI) | String       | N/A            |
| hdrExt             | As in CLI                                      | String       | ```.hpp```     |
| srcExt             | As in CLI                                      | String       | ```.cpp```     |
| moduleInterfaceExt | As in CLI                                      | String       | ```.cppm```    |
| includePaths       | As in CLI                                      | String array | ```[]```       |
| ignoredHdrs        | As in CLI                                      | String array | ```[]```       |

- Transitional modularization settings (mi_ prefix = macro identifier):

| Name                | Description | Value type | Default value      |
|---------------------|-------------|------------|--------------------|
| backCompatHdrs      | As in CLI   | Boolean    | ```false```        |
| mi_control          | As in CLI   | string     | ```CPP_MODULES```  |
| mi_exportKeyword    | As in CLI   | string     | ```EXPORT```       |
| mi_exportBlockBegin | As in CLI   | string     | ```BEGIN_EXPORT``` |
| mi_exportBlockEnd   | As in CLI   | string     | ```END_EXPORT```   |
| exportMacrosPath    | As in CLI   | string     | ```Export.hpp```   |

# Behavior
- A file pair is defined as one header and one with the same basename (filename without extension) in the same directory. For example, ```input/directory/file.cpp``` and ```input/directory/file.hpp
- importizer's behavior is undefined for a header-source pair and the source has a main function. Very rare in real code.
- Action by file type:

| File type | Paired | Has ```main()``` | Conversion                 | Must do manual export |
|-----------|--------|------------------|----------------------------|-----------------------|
| Header    |        | N/A              | Module interface unit      | ✔                     |
| Header    | ✔      | N/A              | Module interface unit      | ✔                     |
| Source    |        |                  | Module interface unit      | ✔                     |
| Source    | ✔      |                  | Module implementation unit |                       |
| Source    |        | ✔                | Only include to import     |                       |

- Behavior of include path searching (similar concept to specifying ```-I```):

| Type          | Action                                                                   |
|---------------|--------------------------------------------------------------------------|
| Quoted        | 1. Relative to directory of the current file<br>2. Same as angle-bracket |
| Angle bracket | 1. Relative to each of ```includePaths```                                |

# Macros
- Modules, unlike headers, were explicitly designed to be encapsulated from macros and so they wouldn't leak macros. If a macro is defined in a header, and the header get modularized, it will only exist in that module only. Some ways to remedy:
  - Add the macro definition on the command line when compiling for the files that needed the macro (using ```-D...```).
  - Refactor the macro definition into a separate header, ```#include``` that where the macro is needed, and add the new header to the ```ignoredHdrs```.
  - Add the macro-containing headers to the ```ignoredHdrs``` (their paired sources, if available will be treated as if they have a ```main()```)

# Some code style rule
- Max column width: 90
- Put comment to denote what type after case label for variant switch
- Keep settings' order in the README, the option struct, and their value-checking order the same.
- To determine order for new settings, optimize for the option struct size