# Test scaffolding shared by umaa-cpp's own tests and downstream projects
# building the SDK from source (build-tree only; deliberately NOT installed).

if(NOT TARGET GTest::gmock)
  find_package(GTest CONFIG REQUIRED)
endif()

add_library(umaa-cpp-test-utils INTERFACE)
add_library(umaa-cpp::test-utils ALIAS umaa-cpp-test-utils)

target_include_directories(umaa-cpp-test-utils INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/include          # LocalReaderSender.h (loopback IO)
  ${CMAKE_CURRENT_SOURCE_DIR}/include/mocks)

target_link_libraries(umaa-cpp-test-utils INTERFACE
  GTest::gmock
  umaa-cpp::umaa-cpp)
