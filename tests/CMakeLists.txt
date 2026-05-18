add_executable(cmpDir CmpDir.cc)
target_compile_features(cmpDir PRIVATE cxx_std_23)
target_link_libraries(cmpDir PRIVATE fmt::fmt)
enable_testing()
set(testResDir "${CMAKE_CURRENT_BINARY_DIR}/TestResults")
function(addTestsFromDir dir)
  file(GLOB testDirs "${dir}/*")
  foreach(testDir ${testDirs})
    if(IS_DIRECTORY ${testDir})
      get_filename_component(testName ${testDir} NAME)
      add_test(NAME "${testName}"
        COMMAND ${CMAKE_COMMAND} -P Driver.cmake "$<TARGET_FILE:importizer>"
        "${testDir}/config.toml" "${testResDir}/${testName}" "$<TARGET_FILE:cmpDir>"
        "${testDir}/CorrectOutput" WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
    endif()
  endforeach()
endfunction(addTestsFromDir)
addTestsFromDir(.)
