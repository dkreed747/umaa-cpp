
# Check if gtest_main exists yet.. if not, define

if(NOT TARGET gtest_main)
  # PROJECT_SOURCE_DIR is umaa-sdk-common
  configure_file(${PROJECT_SOURCE_DIR}/cmake/CMakeLists.txt.gtest.in
    ${CMAKE_BINARY_DIR}/googletest-download/CMakeLists.txt)

  execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )

  execute_process(COMMAND ${CMAKE_COMMAND} --build .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )

  # Prevent overriding parent project's compiler/linker settings
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

  # Add googletest directly to the build.
  add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
                   ${CMAKE_BINARY_DIR}/googletest-build)
endif()

set(LIB_NAME sdk-test-utils)

add_library(${LIB_NAME} INTERFACE)

target_include_directories(${LIB_NAME} INTERFACE
                            ${CMAKE_CURRENT_SOURCE_DIR}/include/mocks
                            )

target_link_libraries(${LIB_NAME} INTERFACE gmock_main umaa-sdk-common)