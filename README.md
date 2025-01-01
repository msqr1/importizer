# include2import
- Convert ```#include``` code to ```import```
- This only convert #include statements to import statement, as well as adding the GMF
- This takes you on the way to modularizing your codebase.
- **You would still have to manually choose what to export after running. You would usually export everything, though.**
- Requires valid C++20

# Usage
- Build from source for now. Just clone, make a directory inside the cloned repo, and run CMake from there.
- Add ```-DCMAKE_BUILD_TYPE=Release``` when running CMake to get an optimized program.
- Requires clang(++) and libc++ 14 or higher, I haven't tested with others yet. You will need to add  ```-DCMAKE_C_COMPILER=clang-14``` and ```-DCMAKE_CXX_COMPILER=clang++-14``` when running CMake
- Create a ```include2import.toml``` in the directory of the generated binary, add some settings, and run the program.
- The output will be a list of file path, relative to ```outDir``` that need to go through manual exporting

# CLI
  - CLI only (flags, one-time settings):
      -  ```-c --config``` - Path to TOML configuration file (```.toml```), default to ```include2import.toml```
      -  ```-h --help``` - Print help and exit
      -  ```-v --version``` - Print version and exit
  - ```.toml```-only settings:
      - ```inDir``` - Input directory (required)
      - ```outDir``` - Output directory (required)
      - ```verbose``` - Enable verbose output (debugging)
      - ```includeGuardPat``` - Regex for name of include guard used in the project (optional) to remove. include2import will try to look for a ```#pragma once``` to remove if unspecified.
      - ```includePaths``` - Include paths searched when converting include to import (optional)
      - ```ignoredHeaders``` - Paths relative to ```inDir``` of header files to ignore (optional). Their paired sources, if available, will be treated as if they have a ```main()```. 
      - ```hdrExt``` - Header file extension
      - ```srcExt``` - Source file (also module implementation unit) extension
      - ```moduleInterfaceExt``` - Module interface unit file extension
  - Default values for ```.toml```-only settings:
      - verbose: ```false``` [boolean]
      - includePaths: ```[]``` [string array]
      - ignoredHeaders: ```[]``` [string array]
      - hdrExt: ```".hpp"``` [string]
      - srcExt: ```".cpp"``` [string]
      - moduleInterfaceExt: ```".cppm"``` [string]
  - Behavior of include path searching (similar concept to specifying ```-I```):
| Type          | Action                                                                   |
|---------------|--------------------------------------------------------------------------|
| Quoted        | 1. Relative to directory of the current file<br>2. Same as angle-bracket |
| Angle bracket | 1. Relative to the include paths listed                                  |

# Behavior
- A file pair is defined as a header and the source with the same basename (filename without extension) in the same directory. For example, ```input/directory/file.cpp``` and ```input/directory/file.hpp```
- Action by file type:
| File type | Paired | Has ```main()``` | Conversion                 | Must do manual export |
|-----------|--------|------------------|----------------------------|-----------------------|
| Header    |        | N/A              | Module interface unit      | ✔                     |
| Header    | ✔      | N/A              | Module interface unit      | ✔                     |
| Source    |        |                  | Module interface unit      | ✔                     |
| Source    | ✔      |                  | Module implementation unit |                       |
| Source    |        | ✔                | Only include to import     |                       |
| Source    | ✔      | ✔                | Only include to import     |                       |

# Macros
- Modules, unlike headers, were explicitly designed to be encapsulated from macros and so they wouldn't leak macros. If a macro is defined in a header, and the header get modularized, it will only exist in that module only. Some ways to remedy:
    - Add the macro definition on the command line when compiling for the files that needed the macro (using ```-D...```).
    - Refactor the macro definition into a separate header, ```#include``` that where the macro is needed, and add the new header to the ```ignoredHeaders```.
    - Add the macro-containing headers to the ```ignoredHeaders``` (their paired sources, if available will be treated as if they have a ```main()```)