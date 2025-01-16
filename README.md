# Introduction
Do you need help converting your header-based codebase to C++20 modules?
include2import can help.

Please note that include2import is still very unstable, but I would love feedback for improvement!

What does it do?
- Convert ```#include``` statements to ```import``` statement, as well as creating the GMF
- Takes you on the way to modularizing your codebase.
- **You would still have to manually choose what to export after running.**

include2import supports two modularization scheme:
- Complete modularization: You want to ditch header-based code altogether and embrace C++ modules. This is the default.
- Transtitional modularization: You want to have both a header-based interface and a module-based one for backward compatibility and a gradual switch. You can opt in by specifing ```[Transitional]``` in the setting file.

# Usage
- Build from source for now. Just clone, make a directory inside the cloned repo, and run CMake from there.
- Add ```-DCMAKE_BUILD_TYPE=Release``` when running CMake to get an optimized program.
- Create a ```include2import.toml``` in the directory of the generated binary (or somewhere else and specify ```-c```), add some settings, and run the program.
- The output will be a list of file path, relative to ```outDir``` that need to go through manual exporting

# CLI
- Some arguments will take precedence over those in the config file
- Paths are relative to the current working directory

| Flag         | Description                                                                         |
|--------------|-------------------------------------------------------------------------------------|
| -c --config  | Path to TOML configuration file (```.toml```), default to ```include2import.toml``` |
| -h --help    | Print help and exit                                                                 |
| -v --version | Print version and exit                                                              |
| -o --outDir  | Output directory (required if not specified in the config file)                     |
| -i --inDir   | Input directory (required if not specified in the config file)                      |

# TOML setting file
- Paths are relative to the config file by default, unless otherwise specified
- Settings are optional, unless otherwise specified
- General settings:

| Setting name       | Description                                                                                                                                 | Value type   | Default value  |
|--------------------|---------------------------------------------------------------------------------------------------------------------------------------------|--------------|----------------|
| inDir              | Input directory (required if not specified on the command line)                                                                             | String       | N/A            |
| outDir             | Output directory (required if not specified on the command line)                                                                            | String       | N/A            |
| hdrExt             | Header file extension                                                                                                                       | String       | ```.hpp```     |
| srcExt             | Source (also module implementation unit) file extension                                                                                     | String       | ```.cpp```     |
| moduleInterfaceExt | Module interface unit file extension                                                                                                        | String       | ```.cppm```    |
| includeGuardPat    | Regex for the include guard identifier used in the project to remove                                                                        | String       | ```[^\s]+_H``` |
| includePaths       | Include paths searched when converting include to import                                                                                    | String array | ```[]```       |
| ignoredHeaders     | Paths relative to ```inDir``` of header files to ignore. Their paired sources, if available, will be treated as if they have a ```main()``` | Boolean      | ```false```    |
| stdInclude2Import  | Convert standard includes to ```import std``` or ```import std.compat```                                                                    | Boolean      | ```false```    |
| logCurrentFile     | Print the current file being processed. This is used when trying to find which file caused problems                                         | Boolean      | ```false```    |

- Transitional modularization settings (mi_ prefix = macro identifier):

| Setting name        | Description                                | Value type | Default value      |
|---------------------|--------------------------------------------|------------|--------------------|
| mi_control          | Header-module switching macro identifier   | string     | ```CPP_MODULES```  |
| mi_exportKeyword    | Exported symbol macro identifier           | string     | ```EXPORT```       |
| mi_exportBlockBegin | Export block begin macro identifier        | string     | ```BEGIN_EXPORT``` |
| mi_exportBlockEnd   | Export block end macro identifier          | string     | ```END_EXPORT```   |
| exportMacrosPath    | Export macros file path relative to outDir | string     | ```Export.hpp```   |

# Behavior
- A file pair is defined as one header and one with the same basename (filename without extension) in the same directory. For example, ```input/directory/file.cpp``` and ```input/directory/file.hpp```. 
- include2import's behavior is undefined for a header-source pair and the source has a main function. This is extremely rare in real code, though.
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
    - Refactor the macro definition into a separate header, ```#include``` that where the macro is needed, and add the new header to the ```ignoredHeaders```.
    - Add the macro-containing headers to the ```ignoredHeaders``` (their paired sources, if available will be treated as if they have a ```main()```)

# Maybe in the future
- Automatic macro refactoring