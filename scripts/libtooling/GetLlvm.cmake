set(v 22.1.6)
set(arFile "LlvmSrc.tar.xz")
file(DOWNLOAD
  https://github.com/llvm/llvm-project/releases/download/llvmorg-${v}/llvm-project-${v}.src.tar.xz
  ${arFile}
  EXPECTED_HASH SHA256=6e0b376a1f6d9873e7dfb09ae6e04b9c7024400f01733fa4c29be69d5c138bc2
  SHOW_PROGRESS STATUS status
)
list(GET status 0 errCode)
if(NOT errCode EQUAL 0)
  list(GET status 1 err)
  message(FATAL_ERROR "Download failed: ${err}")
endif()
file(ARCHIVE_EXTRACT INPUT ${arFile})
file(RENAME llvm-project-${v}.src "llvm-src")
file(REMOVE ${arFile})
