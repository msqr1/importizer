# Acquire the correct options
- I agree with most of the default settings, but my project does use `#pragma once` and `.cc` as the source extension.
- [Configuration file](importizer.toml)
- I also placed the executable and the config file, so the structure look like this:
```
...
├── src
│   └── CMakeLists.txt
├── src2
│   ├── Headers...
│   └── Sources...
├── importizer (executable)
├── importizer.toml
...
```