param (
  [Parameter(Position = 0, Mandatory = $true)]
  [string]$llvmRepo,
  [Parameter(Position = 1, Mandatory = $true)]
  [string]$installDir
)
$ErrorActionPreference = "Stop"
cmake -S "$llvmRepo\llvm" `
  -B "$llvmRepo\build" `
  -G Ninja `
  -DCMAKE_BUILD_TYPE=Release `
  -DLLVM_TARGETS_TO_BUILD=host `
  -DCMAKE_C_COMPILER="clang-cl" `
  -DCMAKE_CXX_COMPILER="clang-cl" `
  -DLLVM_OPTIMIZED_TABLEGEN=on `
  -DLLVM_BUILD_TOOLS=off `
  -DLLVM_ENABLE_PROJECTS="clang" `
  -DLLVM_ENABLE_LTO=Thin `
  -DLLVM_ENABLE_RPMALLOC=on `
  -DLLVM_ENABLE_UNWIND_TABLES=off `
  -DLLVM_ENABLE_ZLIB=off `
  -DLLVM_ENABLE_ZSTD=off `
  -DLLVM_ENABLE_DIA_SDK=off `
  -DLLVM_ENABLE_LIBEDIT=off `
  -DLLVM_ENABLE_LIBPFM=off `
  -DLLVM_ENABLE_BINDINGS=off `
  -DLLVM_APPEND_VC_REV=off `
  -DLLVM_INCLUDE_TOOLS=on `
  -DLLVM_INCLUDE_BENCHMARKS=off `
  -DLLVM_INCLUDE_EXAMPLES=off `
  -DLLVM_INCLUDE_TESTS=off
cmake --build "$llvmRepo\build"
cmake --install "$llvmRepo\build" --prefix "$installDir"
