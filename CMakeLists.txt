#---------------------------------------------------------------------------
#  Copyright 2025 Pennsylvania State University
#
#  Applied Research Laboratory
#  Pennsylvania State University
#  P.O. Box 30
#  State College, PA 16804-0030
#
# DISTRIBUTION STATEMENT A. Approved for public release.
# Distribution is unlimited.
# This software was developed by the Department of the Navy,
# NAVSEA Unmanned and Small Combatants. It is provided under the terms of
# use found in the LICENSE file at the source code root directory.
#---------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.25)

project(umaa-cpp VERSION 1.0.0
        DESCRIPTION "Modern C++ library for building UMAA services over DDS"
        HOMEPAGE_URL "https://gitlab.bongo-barley.ts.net/poseidon/utility/umaa/umaa-cpp"
        LANGUAGES CXX)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(UMAA_CPP_INSTALL "Generate install rules for umaa-cpp" ON)
option(UMAA_CPP_BUILD_TESTS "Build unit tests" OFF)
option(UMAA_CPP_BUILD_DDS_TESTS "Build Cyclone DDS integration tests (needs loopback DDS)" OFF)
option(UMAA_CPP_COVERAGE "Instrument for gcov coverage" OFF)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(PROJECT_IS_TOP_LEVEL)
  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
  if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
  endif()
endif()

# Relocatable install: binaries/libraries find sibling libs wherever the
# prefix is unpacked (same convention as the umaa-cyclone-cpp package).
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)
set(CMAKE_INSTALL_RPATH "$ORIGIN;$ORIGIN/../lib;$ORIGIN/../lib64")

include(GNUInstallDirs)

# ---------------------------------------------------------------------------
# Dependencies (all provided by the umaa-cyclone-cpp development image)
# ---------------------------------------------------------------------------
find_package(umaa_cyclone_cpp REQUIRED)          # UMAA DDS types (/opt/umaa)
find_package(CycloneDDS-CXX CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(GeographicLib CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)   # log4cxx's exported link interface references fmt::fmt
find_package(log4cxx CONFIG REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(uuid REQUIRED IMPORTED_TARGET uuid)

# log4cxx exports a namespaced target in some releases and a bare one in others.
if(TARGET log4cxx::log4cxx)
  set(UMAA_CPP_LOG4CXX_TARGET log4cxx::log4cxx)
else()
  set(UMAA_CPP_LOG4CXX_TARGET log4cxx)
endif()

# ---------------------------------------------------------------------------
# The umaa-cpp library
# ---------------------------------------------------------------------------
add_library(umaa-cpp
  src/config/AppConfig.cpp
  src/config/ConfigurationManager.cpp
  src/common/SystemClock.cpp
  src/io/dds/cyclone/CycloneQosProviderWrapper.cpp
  src/common/UuidFactory.cpp
  src/umaa/domain/AsyncCommandSession.cpp
  src/umaa/domain/UmaaCommandSession.cpp
  src/umaa/UmaaUtils.cpp
  src/health-status/HealthStatusControl.cpp
  src/log-service/LogReportService.cpp
  src/umaa/CommandStateMachine.cpp
  src/umaa/TaskStateMachine.cpp
  src/umaa/conditionals/ConditionalFactory.cpp
  src/umaa/services/ActiveConstraintsControlProvider.cpp
  src/umaa/services/ConditionalReportConsumer.cpp
  src/umaa/services/ConditionalReportProvider.cpp
  src/umaa/objective-executors/core/ObjectiveStateMachine.cpp
  src/umaa/objective-executors/services/GlobalWaypointControlServiceConsumer.cpp
  src/umaa/objective-executors/services/ObjectiveExecutorControlServiceProvider.cpp
  src/umaa/objective-executors/services/ObjectiveExecutorStateControlServiceProvider.cpp
  src/umaa/objective-executors/ObjectiveController.cpp
  src/domain/CovarianceAttitudeRateType.cpp
  src/domain/CovarianceOrientationType.cpp
  src/domain/CovariancePositionNEDType.cpp
  src/domain/CovarianceVelocityType.cpp
  src/domain/GlobalPoseData.cpp
  src/domain/GlobalPoseAttitude.cpp
  src/domain/OrientationVel3D.cpp
  src/domain/SpeedStatusData.cpp
  src/domain/Velocity3DPlatformNEDType.cpp
  src/domain/VelocityStatusData.cpp
  src/io/udp/NetworkUtilities.cpp
  src/io/udp/UdpThread.cpp
  src/umaa/specializations/factory/ObjectiveSpecFactory.cpp
)
add_library(umaa-cpp::umaa-cpp ALIAS umaa-cpp)

target_compile_features(umaa-cpp PUBLIC cxx_std_20)
set_target_properties(umaa-cpp PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
  EXPORT_NAME umaa-cpp)

# The codebase uses flat includes (e.g. #include "AppConfig.h"), so every
# include/ subdirectory is exported on the interface. The header tree installs
# verbatim under include/umaa-cpp/. (Migrating to pathed includes like
# <umaa-cpp/config/AppConfig.h> is a planned follow-up.)
set(UMAA_CPP_INCLUDE_SUBDIRS
  ""
  algorithms
  algorithms/guidance
  common
  config
  domain
  env
  health-status
  io
  io/dds
  io/dds/base
  io/dds/cyclone
  io/udp
  log-service
  observer
  umaa
  umaa/conditionals
  umaa/domain
  umaa/io
  umaa/objective-executors
  umaa/objective-executors/core
  umaa/objective-executors/io
  umaa/objective-executors/services
  umaa/services
  umaa/services/base
  umaa/specializations
  umaa/specializations/data
  umaa/specializations/factory
  umaa/specializations/utils
)
foreach(_dir IN LISTS UMAA_CPP_INCLUDE_SUBDIRS)
  target_include_directories(umaa-cpp PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/${_dir}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/umaa-cpp/${_dir}>")
endforeach()

target_link_libraries(umaa-cpp
  PUBLIC
    umaa_cyclone_cpp::umaa_cyclone_cpp   # UMAA/*.hpp in public umaa/ headers
    CycloneDDS-CXX::ddscxx               # dds/dds.hpp in public io/dds headers
    yaml-cpp::yaml-cpp                   # yaml-cpp/yaml.h in ConfigurationManager.h
    GeographicLib::GeographicLib         # GeographicLib/*.hpp in algorithms/ headers
    ${UMAA_CPP_LOG4CXX_TARGET}           # log4cxx in Logger.h
  PRIVATE
    PkgConfig::uuid                      # uuid/uuid.h only in UuidFactory.cpp
)

if(UMAA_CPP_COVERAGE)
  target_compile_options(umaa-cpp PRIVATE --coverage)
  target_link_options(umaa-cpp PUBLIC --coverage)
endif()

# Runtime config next to the build tree for developer convenience.
configure_file(config/CYCLONE_QOS_PROFILES.xml ${CMAKE_BINARY_DIR}/CYCLONE_QOS_PROFILES.xml COPYONLY)
configure_file(config/log4cxx.xml ${CMAKE_BINARY_DIR}/log4cxx.xml COPYONLY)

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
if(UMAA_CPP_BUILD_TESTS OR UMAA_CPP_BUILD_DDS_TESTS)
  if(PROJECT_IS_TOP_LEVEL)
    enable_testing()
  endif()
  add_subdirectory(test-utils)
  add_subdirectory(test)
endif()

# ---------------------------------------------------------------------------
# Install / export (find_package(umaa-cpp) for standalone consumers)
# ---------------------------------------------------------------------------
if(UMAA_CPP_INSTALL)
  install(TARGETS umaa-cpp EXPORT umaa-cppTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT runtime
            NAMELINK_COMPONENT development
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT development
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

  install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/umaa-cpp
    COMPONENT development
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")

  install(FILES
      config/CYCLONE_QOS_PROFILES.xml
      config/cyclonedds-loopback.xml
      config/log4cxx.xml
      config/system-config.yml
    DESTINATION ${CMAKE_INSTALL_DATADIR}/umaa-cpp/config COMPONENT runtime)

  install(EXPORT umaa-cppTargets
    FILE umaa-cppTargets.cmake
    NAMESPACE umaa-cpp::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/umaa-cpp
    COMPONENT development)

  include(CMakePackageConfigHelpers)
  configure_package_config_file(cmake/umaa-cppConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/umaa-cppConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/umaa-cpp)
  write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/umaa-cppConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)
  install(FILES
      ${CMAKE_CURRENT_BINARY_DIR}/umaa-cppConfig.cmake
      ${CMAKE_CURRENT_BINARY_DIR}/umaa-cppConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/umaa-cpp
    COMPONENT development)
endif()
