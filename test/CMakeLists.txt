add_executable(test Driver.cc)
add_subdirectory(../3rdParty/Catch2 "${CMAKE_BINARY_DIR}/Catch2")
target_link_libraries(test PRIVATE Catch2::Catch2WithMain)