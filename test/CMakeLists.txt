add_executable(cmpdir CmpDir.cc)
add_executable(gettmpdir GetTmpDir.cc)
target_compile_features(cmpdir PRIVATE cxx_std_23)
target_compile_features(gettmpdir PRIVATE cxx_std_17)
target_compile_options(cmpdir PRIVATE -Wall -Wextra)
target_compile_options(gettmpdir PRIVATE -Wall -Wextra)
target_link_libraries(cmpdir fmt::fmt)
target_link_libraries(gettmpdir fmt::fmt)
enable_testing()

# Find a way to gettmpdir to put in tmpDir

function(addTestsFromDir dir)
  file(GLOB testDirs "${dir}/*")
  file(REMOVE_RECURSE ${tmpDir})
  file(MAKE_DIRECTORY ${tmpDir})
  foreach(testDir ${testDirs})
    get_filename_component(testName ${testDir} NAME)
    add_test(NAME "${dir}.${testName}" 
      COMMAND ${CMAKE_COMMAND} -P Driver.cmake $<TARGET_FILE:include2import> 
      "${testDir}/config.toml" "${tmpDir}/${dir}/${testName}" $<TARGET_FILE:cmpdir> 
      "${testDir}/CorrectOutput" WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  endforeach()
endfunction(addTestsFromDir)

addTestsFromDir(Default)
addTestsFromDir(Transitional)
