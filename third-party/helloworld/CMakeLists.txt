project(hello-world-types LANGUAGES C CXX)
cmake_minimum_required(VERSION 3.16)

if(NOT TARGET CycloneDDS-CXX::ddscxx)
  find_package(CycloneDDS-CXX REQUIRED)
endif()

set(OUTPUT_FILES ${PROJECT_BINARY_DIR}/HelloWorldData.cpp ${PROJECT_BINARY_DIR}/HelloWorldData.hpp)

# Generate header files at configure time 
execute_process(COMMAND idlc -l cxx ${CMAKE_CURRENT_SOURCE_DIR}/HelloWorldData.idl -o ${PROJECT_BINARY_DIR}
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

idlcxx_generate(TARGET helloworlddata 
      FILES ${CMAKE_CURRENT_SOURCE_DIR}/HelloWorldData.idl 
      WARNINGS no-implicit-extensibility
      OUTPUT_DIR ${PROJECT_BINARY_DIR}
)

include_directories(
    ${PROJECT_BINARY_DIR}
)

add_library(hello-world-types ${PROJECT_BINARY_DIR}/HelloWorldData.cpp ${PROJECT_BINARY_DIR}/HelloWorldData.hpp)

# Link both executables to idl data type library and ddscxx.
target_link_libraries(hello-world-types CycloneDDS-CXX::ddscxx helloworlddata)

set_property(TARGET hello-world-types PROPERTY CXX_STANDARD 17)


