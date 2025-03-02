# Background
## Motivation
I want more people to use modules because they were brought in to address problems that comes with the `include` directive (that I hate):
- **Compilation time:** `include`s will be expanded recursively, meaning `include`s inside `include` will also be expanded, and so on. With just `#include <iostream>`, GCC 14.2 will produce a file with 30914 lines of code. If you use include `iostream` in many files, the compiler will process that exact same 30914 lines that many times. In contrast, modules don't need to be recompiled every time it is imported, it is compiled exactly once and reused, avoiding redudant processing.
- **Encapsulation:** Since `include`s are just copying and pasting, it will expose everything inside the included files, whether you want it or not. In contrast, modules allow you to choose what is exposed to users (exported). Macros, `include`s, entities from other imported modules and non-exported entities from a module will strictly stay in that module.
- **Cleaner syntax:** No need for include guards, pragma once.
Modularization is cumbersome. Not only we need to replace `include`s with `import`s, we also have to 

## Secondary motivation


# Overview