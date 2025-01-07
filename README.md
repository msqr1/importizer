# include2import
- Still very unstable, but I would love feedback for improvement!
- Convert ```#include``` code to ```import```
- This only convert #include statements to import statement, as well as creating the GMF
- This takes you on the way to modularizing your codebase.
- **You would still have to manually choose what to export after running. You would usually export everything, though.**
- Requires valid C++20

# Usage
- Build from source for now. Just clone, make a directory inside the cloned repo, and run CMake from there.
- Add ```-DCMAKE_BUILD_TYPE=Release``` when running CMake to get an optimized program.
- Requires libc++16 or higher and the clang compiler, I haven't tested with others yet.
- Create a ```include2import.toml``` in the directory of the generated binary (or somewhere else and specify ```-c```), add some settings, and run the program.
- The output will be a list of file path, relative to ```outDir``` that need to go through manual exporting

# CLI
  - CLI only (flags, one-time settings):
      -  ```-c --config``` - Path to TOML configuration file (```.toml```), default to ```include2import.toml```
      -  ```-h --help``` - Print help and exit
      -  ```-v --version``` - Print version and exit
  - Required ```.toml```-only settings:
      - ```inDir``` - Input directory relative to the config file
      - ```outDir``` - Output directory relative to the config file
  - Optional ```.toml```-only settings:
      - ```verbose``` - Enable verbose output for debugging
      - ```hdrExt``` - Header file extension
      - ```srcExt``` - Source file (also module implementation unit) extension
      - ```moduleInterfaceExt``` - Module interface unit file extension
      - ```includeGuardPat``` - Regex for name of include guard used in the project to remove. include2import will try to look for a ```#pragma once``` to remove if unspecified.
      - ```includePaths``` - Include paths relative to the config file searched when converting include to import
      - ```ignoredHeaders``` - Paths relative to ```inDir``` of header files to ignore. Their paired sources, if available, will be treated as if they have a ```main()```
      - ```stdInclude2Import``` - Convert standard includes to ```import std``` or ```import std.compat```
  - Default values for optional ```.toml```-only settings:
      - verbose: ```false``` [boolean]
      - hdrExt: ```".hpp"``` [string]
      - srcExt: ```".cpp"``` [string]
      - moduleInterfaceExt: ```".cppm"``` [string]
      - includeGuardPat: (nothing) [string]
      - includePaths: ```[]``` [string array]
      - ignoredHeaders: ```[]``` [string array]
      - stdInclude2Import: ```false``` [boolean]
  - Behavior of include path searching (similar concept to specifying ```-I```):

| Type          | Action                                                                   |
|---------------|--------------------------------------------------------------------------|
| Quoted        | 1. Relative to directory of the current file<br>2. Same as angle-bracket |
| Angle bracket | 1. Relative to the include paths listed                                  |

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

# Macros
- Modules, unlike headers, were explicitly designed to be encapsulated from macros and so they wouldn't leak macros. If a macro is defined in a header, and the header get modularized, it will only exist in that module only. Some ways to remedy:
    - Add the macro definition on the command line when compiling for the files that needed the macro (using ```-D...```).
    - Refactor the macro definition into a separate header, ```#include``` that where the macro is needed, and add the new header to the ```ignoredHeaders```.
    - Add the macro-containing headers to the ```ignoredHeaders``` (their paired sources, if available will be treated as if they have a ```main()```)

# Maybe in the future
- Automatic macro refactoring