add_executable(cmp-dir CmpDir.cc)
target_compile_features(cmp-dir PRIVATE cxx_std_23)
target_link_libraries(cmp-dir PRIVATE fmt::fmt)
target_compile_options(cmp-dir PRIVATE
  -Wall
  -fno-rtti
)
enable_testing()

# Just version for now, will evolve into a full opt parsing test
add_test(NAME "Version" COMMAND "$<TARGET_FILE:importizer>" -v)
set_tests_properties("Version" PROPERTIES PASS_REGULAR_EXPRESSION "^3\\.0\\.0\n$")

set(testResDir "${CMAKE_CURRENT_BINARY_DIR}/TestResults")
function(addFunctionalTests dir)
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
          -DCORRECT="${testDir}/Correct"
        )
      set_tests_properties("${testName}" PROPERTIES DEPENDS "Version")
    endif()
  endforeach()
endfunction()
addFunctionalTests(.)
