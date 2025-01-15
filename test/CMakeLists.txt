add_executable(cmpdir CmpDir.cc)
target_compile_features(cmpdir PRIVATE cxx_std_23)
target_compile_options(cmpdir PRIVATE -Wall -Wextra)
target_link_libraries(cmpdir fmt::fmt)
enable_testing()
set(testResDir "${CMAKE_CURRENT_BINARY_DIR}/results")
function(addTestsFromDir dir)
  file(GLOB testDirs "${dir}/*")
  foreach(testDir ${testDirs})
    get_filename_component(testName ${testDir} NAME)
    add_test(NAME "${dir}.${testName}" 
      COMMAND ${CMAKE_COMMAND} -P Driver.cmake $<TARGET_FILE:include2import> 
      "${testDir}/config.toml" "${testResDir}/${dir}/${testName}" $<TARGET_FILE:cmpdir> 
      "${testDir}/CorrectOutput" WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  endforeach()
endfunction(addTestsFromDir)

addTestsFromDir(Default)
addTestsFromDir(Transitional)
