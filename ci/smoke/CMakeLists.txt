# Minimal standalone consumer of the installed umaa-cpp package.
#
# Used by CI to prove the SDK is consumable with nothing but
# find_package(umaa-cpp) — both from the SDK container image (zero-setup via
# CMAKE_PREFIX_PATH) and from a freshly unpacked package tarball. Also handy
# locally:
#
#   cmake -S ci/smoke -B build-smoke -DCMAKE_PREFIX_PATH=<sdk-prefix>
#   cmake --build build-smoke
#   CYCLONEDDS_URI=file://$PWD/config/cyclonedds-loopback.xml ./build-smoke/umaa-cpp-smoke

cmake_minimum_required(VERSION 3.25)
project(umaa-cpp-smoke LANGUAGES CXX)

find_package(umaa-cpp CONFIG REQUIRED)

add_executable(umaa-cpp-smoke main.cpp)
target_link_libraries(umaa-cpp-smoke PRIVATE umaa-cpp::umaa-cpp)
