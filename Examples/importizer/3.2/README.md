# 1. Add export
- Just add some export, there are forward declaration here and there, so I avoid that.
- The gist of it is only export things that other modules (or users for API) need, and nothing else!

# 2. Test-compile the modularized project
- With the context of the project using CMake, I modify the `CMakeLists.txt` to setup a module build.
- CMake also have very good module support, featuring automatic dependency scanning, so I highly recommend it for now.
- Since importizer uses CMake 3.25 minimum, and CMake officially support modules from 3.28. I will have to raise the minimum version (in the top-level `CMakeLists.txt`). 
- Note that you can still try working around it for CMake from 3.25 - 3.27.9 with the experimental API, but to me that is just too much.
- The complete picture to build is in the importizer directory, now we just do the normal build process. You can copy the importizer folder here to build it for yourself. Make sure you have a module-supporting compiler and generator.
```sh
mkdir build
cd build

# For my machine
cmake .. -DCMAKE_CXX_COMPILER=clang++-19 -DCMAKE_C_COMPILER=clang-19 -DCMAKE_CXX_FLAGS="-stdlib=libc++" -G "Ninja"
cmake --build .
```