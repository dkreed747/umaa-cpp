# Sample IDL type library used by the Cyclone DDS integration tests.
# Added via add_subdirectory() from test/CMakeLists.txt.
#
# idlcxx_generate() creates an INTERFACE target that only carries the generated
# .cpp/.hpp (it does not compile them). Generate here, then compile the sources
# into a real library (same pattern as the umaa-cyclone-cpp types build).

if(NOT TARGET CycloneDDS-CXX::ddscxx)
  find_package(CycloneDDS-CXX REQUIRED)
endif()

idlcxx_generate(
  TARGET helloworlddata_gen
  FILES ${CMAKE_CURRENT_SOURCE_DIR}/HelloWorldData.idl
  OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}
  WARNINGS no-implicit-extensibility)

get_target_property(_gen_all helloworlddata_gen INTERFACE_SOURCES)
set(_gen_cpp ${_gen_all})
list(FILTER _gen_cpp INCLUDE REGEX "\\.cpp$")

add_library(hello-world-types ${_gen_cpp})
add_dependencies(hello-world-types helloworlddata_gen_generate)
target_include_directories(hello-world-types PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(hello-world-types PUBLIC CycloneDDS-CXX::ddscxx)
