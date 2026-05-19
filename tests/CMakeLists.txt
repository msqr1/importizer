add_executable(cmp-dir CmpDir.cc)
target_compile_features(cmp-dir PRIVATE cxx_std_23)
target_link_libraries(cmp-dir PRIVATE fmt::fmt)
target_compile_options(cmp-dir PRIVATE
  -Wall
  -fno-rtti
)
target_link_options(cmp-dir PRIVATE
  -static
)
enable_testing()
set(testResDir "${CMAKE_CURRENT_BINARY_DIR}/TestResults")
function(addTestsFromDir dir)
  file(GLOB testDirs "${dir}/*")
  foreach(testDir ${testDirs})
    if(IS_DIRECTORY ${testDir})
      get_filename_component(testName ${testDir} NAME)
      add_test(NAME "${testName}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMAND ${CMAKE_COMMAND} -P Driver.cmake
        -DIMPORTIZER="$<TARGET_FILE:importizer>"
        -DCONFIG="${testDir}/config.toml"
        -DOUT_DIR="${testResDir}/${testName}"
        -DCMP_DIR="$<TARGET_FILE:cmp-dir>"
        -DCORRECT_OUTPUT="${testDir}/CorrectOutput"
        )
    endif()
  endforeach()
endfunction(addTestsFromDir)
addTestsFromDir(.)
