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

cmake_minimum_required (VERSION 3.16)

project(umaa-sdk-common VERSION 1.0.0
        DESCRIPTION "Common Libraries for UMAA-SDK"
        LANGUAGES CXX)

# Sets Path to helper modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Set UMAA Version
set(UMAA_VERSION 6.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(COMPILE_OPTS)

# Options for Building Tests
# Default value is OFF
option(TEST_COMMON "TEST_COMMON")
option(CODE_COVERAGE "GCOV_CODE_COVERAGE")
option(TEST_CYCLONE "TEST_CYCLONE")

set(BASE_INCLUDE_DIRS
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/include/algorithms
  ${PROJECT_SOURCE_DIR}/include/algorithms/guidance
  ${PROJECT_SOURCE_DIR}/include/common
  ${PROJECT_SOURCE_DIR}/include/domain
  ${PROJECT_SOURCE_DIR}/include/observer
  ${PROJECT_SOURCE_DIR}/include/config
  ${PROJECT_SOURCE_DIR}/include/umaa
  ${PROJECT_SOURCE_DIR}/include/umaa/conditionals
  ${PROJECT_SOURCE_DIR}/include/umaa/domain
  ${PROJECT_SOURCE_DIR}/include/umaa/io
  ${PROJECT_SOURCE_DIR}/include/umaa/objective-executors
  ${PROJECT_SOURCE_DIR}/include/umaa/objective-executors/core
  ${PROJECT_SOURCE_DIR}/include/umaa/objective-executors/io
  ${PROJECT_SOURCE_DIR}/include/umaa/objective-executors/services
  ${PROJECT_SOURCE_DIR}/include/umaa/specializations
  ${PROJECT_SOURCE_DIR}/include/umaa/specializations/data
  ${PROJECT_SOURCE_DIR}/include/umaa/specializations/factory
  ${PROJECT_SOURCE_DIR}/include/umaa/specializations/utils
  ${PROJECT_SOURCE_DIR}/include/umaa/services
  ${PROJECT_SOURCE_DIR}/include/umaa/services/base
  ${PROJECT_SOURCE_DIR}/include/io
  ${PROJECT_SOURCE_DIR}/include/io/dds/
  ${PROJECT_SOURCE_DIR}/include/io/dds/base
  ${PROJECT_SOURCE_DIR}/include/io/dds/cyclone
  ${PROJECT_SOURCE_DIR}/include/io/dds/util
  ${PROJECT_SOURCE_DIR}/include/io/udp/
  ${PROJECT_SOURCE_DIR}/include/env
  ${PROJECT_SOURCE_DIR}/include/health-status
  ${PROJECT_SOURCE_DIR}/include/log-service
  ${PROJECT_SOURCE_DIR}/third-party/yaml-cpp/include
)

set(CYCLONE_INCLUDE_DIRS
  /usr/local/include/ddscxx
)

set(UMAA_INCLUDES 
    /usr/local/umaa-${UMAA_VERSION}
)

set(GEOGRAPHIC_LIB_INCLUDES
  /usr/include/GeographicLib
)

# Compile yaml-cpp library
set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "Option for Building tests" FORCE)
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "Option for Building tools" FORCE)
add_subdirectory(${PROJECT_SOURCE_DIR}/third-party/yaml-cpp ${CMAKE_CURRENT_BINARY_DIR}/yaml-cpp)

find_package(CycloneDDS-CXX CONFIG REQUIRED)
find_package(Log4CXX REQUIRED)
find_package(GeographicLib REQUIRED)

include_directories(
  ${BASE_INCLUDE_DIRS}
  ${CYCLONE_INCLUDE_DIRS}
  ${UMAA_INCLUDES}
)

# Core libraries for UMAA SDK
add_library(umaa-sdk-common
  ${PROJECT_SOURCE_DIR}/src/config/AppConfig.cpp
  ${PROJECT_SOURCE_DIR}/src/config/ConfigurationManager.cpp
  ${PROJECT_SOURCE_DIR}/src/common/SystemClock.cpp
  ${PROJECT_SOURCE_DIR}/src/io/dds/cyclone/CycloneQosProviderWrapper.cpp
  ${PROJECT_SOURCE_DIR}/src/common/UuidFactory.cpp
  ${PROJECT_SOURCE_DIR}/src/umaa/domain/AsyncCommandSession.cpp
  ${PROJECT_SOURCE_DIR}/src/umaa/domain/UmaaCommandSession.cpp
  ${PROJECT_SOURCE_DIR}/src/umaa/UmaaUtils.cpp
  ${PROJECT_SOURCE_DIR}/src/health-status/HealthStatusControl.cpp
  ${PROJECT_SOURCE_DIR}/src/log-service/LogReportService.cpp 
  ${PROJECT_SOURCE_DIR}/src/umaa/CommandStateMachine.cpp
  ${PROJECT_SOURCE_DIR}/src/umaa/TaskStateMachine.cpp
  ${PROJECT_SOURCE_DIR}/src/umaa/conditionals/ConditionalFactory.cpp
  ${PROJECT_SOURCE_DIR}/src/umaa/services/ActiveConstraintsControlProvider.cpp
  ${PROJECT_SOURCE_DIR}/src/umaa/services/ConditionalReportConsumer.cpp
  ${PROJECT_SOURCE_DIR}/src/umaa/services/ConditionalReportProvider.cpp
  ${PROJECT_SOURCE_DIR}/src/umaa/objective-executors/core/ObjectiveStateMachine.cpp
  ${PROJECT_SOURCE_DIR}/src/umaa/objective-executors/services/GlobalWaypointControlServiceConsumer.cpp
  ${PROJECT_SOURCE_DIR}/src/umaa/objective-executors/services/ObjectiveExecutorControlServiceProvider.cpp
  ${PROJECT_SOURCE_DIR}/src/umaa/objective-executors/services/ObjectiveExecutorStateControlServiceProvider.cpp
  ${PROJECT_SOURCE_DIR}/src/umaa/objective-executors/ObjectiveController.cpp
  ${PROJECT_SOURCE_DIR}/src/domain/CovarianceAttitudeRateType.cpp
  ${PROJECT_SOURCE_DIR}/src/domain/CovarianceOrientationType.cpp
  ${PROJECT_SOURCE_DIR}/src/domain/CovariancePositionNEDType.cpp
  ${PROJECT_SOURCE_DIR}/src/domain/CovarianceVelocityType.cpp
  ${PROJECT_SOURCE_DIR}/src/domain/GlobalPoseData.cpp
  ${PROJECT_SOURCE_DIR}/src/domain/GlobalPoseAttitude.cpp
  ${PROJECT_SOURCE_DIR}/src/domain/OrientationVel3D.cpp
  ${PROJECT_SOURCE_DIR}/src/domain/SpeedStatusData.cpp
  ${PROJECT_SOURCE_DIR}/src/domain/Velocity3DPlatformNEDType.cpp
  ${PROJECT_SOURCE_DIR}/src/domain/VelocityStatusData.cpp
  ${PROJECT_SOURCE_DIR}/src/io/udp/NetworkUtilities.cpp
  ${PROJECT_SOURCE_DIR}/src/io/udp/UdpThread.cpp
  ${PROJECT_SOURCE_DIR}/src/umaa/specializations/factory/ObjectiveSpecFactory.cpp
)

target_include_directories(umaa-sdk-common PUBLIC
  ${BASE_INCLUDE_DIRS}
  ${CYCLONE_INCLUDE_DIRS}
  ${GEOGRAPHIC_LIB_INCLUDES}
  PRIVATE ${UMAA_INCLUDES}
 )


# Libraries containing UMAA CXX Implementations
add_library(umaa++
  ${PROJECT_SOURCE_DIR}/src/umaa/UmaaUtils.cpp
  ${PROJECT_SOURCE_DIR}/src/umaa/CommandStateMachine.cpp
  ${PROJECT_SOURCE_DIR}/src/umaa/TaskStateMachine.cpp
  ${PROJECT_SOURCE_DIR}/src/umaa/objective-executors/core/ObjectiveStateMachine.cpp
  ${PROJECT_SOURCE_DIR}/src/umaa/objective-executors/services/GlobalWaypointControlServiceConsumer.cpp
  ${PROJECT_SOURCE_DIR}/src/umaa/objective-executors/services/ObjectiveExecutorControlServiceProvider.cpp
  ${PROJECT_SOURCE_DIR}/src/umaa/objective-executors/services/ObjectiveExecutorStateControlServiceProvider.cpp
  ${PROJECT_SOURCE_DIR}/src/umaa/objective-executors/ObjectiveController.cpp
)

target_include_directories(umaa++ PUBLIC
  ${BASE_INCLUDE_DIRS}
  ${CYCLONE_INCLUDE_DIRS}
  PRIVATE ${UMAA_INCLUDES}
)

set(COMPILE_OPTS ${COMPILE_OPTS} -fPIC -fpermissive)

set(COMMON_LIB_DEPS
      yaml-cpp
      LOG4CXX
      uuid
      CycloneDDS-CXX::ddscxx
      GeographicLib
      umaa-cyclone-cxx-types
)

if(TEST_COMMON OR TEST_CYCLONE)
  enable_testing()
  add_subdirectory(test)
endif()

if(CODE_COVERAGE)
  set(TEST_LIBS ${TEST_LIBS} gcov)
  set(COMPILE_OPTS ${COMPILE_OPTS} -fprofile-arcs -ftest-coverage)
endif()

configure_file(${PROJECT_SOURCE_DIR}/config/CYCLONE_QOS_PROFILES.xml ${CMAKE_BINARY_DIR}/CYCLONE_QOS_PROFILES.xml COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/config/log4cxx.xml ${CMAKE_BINARY_DIR}/log4cxx.xml COPYONLY)

target_compile_options(umaa++ PRIVATE ${COMPILE_OPTS})
target_link_libraries(umaa++ PUBLIC ${COMMON_LIB_DEPS})
target_compile_options(umaa-sdk-common PRIVATE ${COMPILE_OPTS})
target_link_libraries(umaa-sdk-common ${COMMON_LIB_DEPS})
