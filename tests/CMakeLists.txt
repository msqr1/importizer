CPMAddPackage("gh:sheredom/subprocess.h#main")
add_executable(test Test.cc)
target_compile_features(test PRIVATE cxx_std_23)
target_link_libraries(test PRIVATE fmt::fmt)
target_compile_options(test PRIVATE
  -Wall
  -fno-rtti
  $<$<CONFIG:Debug>:-O0 -fno-omit-frame-pointer -fsanitize=undefined -fsanitize=address>
  $<$<CONFIG:Release>:-flto>
)
target_link_options(test PRIVATE
  $<$<CONFIG:Debug>:-O0 -fsanitize=undefined -fsanitize=address>
  $<$<CONFIG:Release>:-flto>
)
target_include_directories(test PRIVATE ${subprocess.h_SOURCE_DIR})
enable_testing()

file(GLOB testDirs "./*")
foreach(testDir ${testDirs})
  if(NOT IS_DIRECTORY ${testDir})
    continue()
  endif()
  get_filename_component(testName ${testDir} NAME)
  add_test(
    NAME "${testName}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMAND "$<TARGET_FILE:test>"
      "$<TARGET_FILE:importizer>"
      "${testName}"
      "${CMAKE_CURRENT_BINARY_DIR}/${testName}"
  )
endforeach()
