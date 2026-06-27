#---------------------------------------------------------------------------
# Applied Research Laboratory
# Pennsylvania State University
# P.O. Box 30
# State College, PA 16804-0030
#
# Copyright 2022 The Pennsylvania State University
#---------------------------------------------------------------------------

#-----------------------------------------#
#              Add gtest                  #
#-----------------------------------------#

message("Building UMAA-SDK Common Tests")

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND (TEST_COMMON OR TEST_CYCLONE))

  #-----------------------------------------------------------------------------------------------------------#
  # https://github.com/google/googletest/tree/master/googletest#incorporating-into-an-existing-cmake-project  #
  #-----------------------------------------------------------------------------------------------------------#

  # Download and unpack googletest at configure time
  if(NOT TARGET gtest_main)
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

# Add Test Utils subdirectory if it hasn't been defined yet
if(NOT TARGET sdk_test_utils)
  add_subdirectory(${PROJECT_SOURCE_DIR}/test-utils ${CMAKE_BINARY_DIR}/test-utils)
endif()

  # Initial unit test library dependencies
  set(TEST_LIBS 
      gtest_main
      yaml-cpp
      LOG4CXX
      uuid
      sdk-test-utils)

  if(CODE_COVERAGE)
    set(TEST_LIBS ${TEST_LIBS} gcov)
    set(COMPILE_OPTS ${COMPILE_OPTS} -fprofile-arcs -ftest-coverage)
  endif()

  configure_file(${PROJECT_SOURCE_DIR}/config/CYCLONE_QOS_PROFILES.xml ${CMAKE_CURRENT_BINARY_DIR}/CYCLONE_QOS_PROFILES.xml COPYONLY)
  configure_file(${PROJECT_SOURCE_DIR}/config/log4cxx.xml ${CMAKE_CURRENT_BINARY_DIR}/log4cxx.xml COPYONLY)

  #------------------------------#
  #    UMAA SDK Common Tests     #
  #------------------------------#
  if (TEST_COMMON)

    #------------------------------#
    #      SDK Common Tests        #
    #------------------------------#
    # These test non UMAA implementations 
    add_executable(umaa_sdk_common_test
    ${PROJECT_SOURCE_DIR}/test/domain/GlobalPoseData_test.cpp
    ${PROJECT_SOURCE_DIR}/test/domain/SpeedStatusData_test.cpp
    ${PROJECT_SOURCE_DIR}/test/domain/BatteryMgmtStatusData_test.cpp
    ${PROJECT_SOURCE_DIR}/test/observer/InputConnector_test.cpp
    ${PROJECT_SOURCE_DIR}/test/common/RealtimeSystemClock_test.cpp
    ${PROJECT_SOURCE_DIR}/test/common/UuidBase_test.cpp
    ${PROJECT_SOURCE_DIR}/test/io/udp/NetworkUtilities_test.cpp
    ${PROJECT_SOURCE_DIR}/test/io/dds/base/BufferedReaderBase_test.cpp
    ${PROJECT_SOURCE_DIR}/test/io/dds/IOReaderRegistryTest.cpp
    ${PROJECT_SOURCE_DIR}/test/io/dds/IOWriterRegistryTest.cpp
    ${PROJECT_SOURCE_DIR}/test/env/Env_Test.cpp)

    target_include_directories(umaa_sdk_common_test PUBLIC
      ${BASE_INCLUDE_DIRS}
      ${CYCLONE_INCLUDE_DIRS}
      ${PROJECT_SOURCE_DIR}/test/io/dds/util)

    target_link_libraries(umaa_sdk_common_test ${TEST_LIBS} umaa-sdk-common CycloneDDS-CXX::ddscxx umaa-cyclone-cxx-types)
    target_compile_options(umaa_sdk_common_test PRIVATE ${COMPILE_OPTS})

    ## Add Tests to be executed
    add_test(NAME umaa_sdk_common_test COMMAND umaa_sdk_common_test --gtest_output=xml:test-reports/umaa_sdk_common_test_report.xml)

    #------------------------------#
    #    UMAA SDK Common Tests     #
    #------------------------------#
    # These test UMAA implementations
    add_executable(umaa_cxx_test
      ${PROJECT_SOURCE_DIR}/test/observer/Subject_test.cpp
      ${PROJECT_SOURCE_DIR}/test/config/ConfigurationManager_test.cpp
      ${PROJECT_SOURCE_DIR}/test/log-service/LogReportService_test.cpp   
      ${PROJECT_SOURCE_DIR}/test/health-status/HealthStatusControl_test.cpp
      ${PROJECT_SOURCE_DIR}/test/io/dds/base/ObservableReader_test.cpp
      ${PROJECT_SOURCE_DIR}/test/io/dds/util/LocalReaderSender_test.cpp
      ${PROJECT_SOURCE_DIR}/test/algorithms/GeographicUtils_test.cpp
      ${PROJECT_SOURCE_DIR}/test/algorithms/VectorUtils_test.cpp
      ${PROJECT_SOURCE_DIR}/test/umaa/CommandStateMachine_test.cpp
      ${PROJECT_SOURCE_DIR}/test/umaa/TaskStateMachine_test.cpp
      ${PROJECT_SOURCE_DIR}/test/umaa/DestinationReaderFilter_test.cpp
      ${PROJECT_SOURCE_DIR}/test/umaa/LargeList_test.cpp
      ${PROJECT_SOURCE_DIR}/test/umaa/LargeListWriter_test.cpp
      ${PROJECT_SOURCE_DIR}/test/umaa/LargeListReader_test.cpp
      ${PROJECT_SOURCE_DIR}/test/umaa/LargeSet_test.cpp
      ${PROJECT_SOURCE_DIR}/test/umaa/LargeSetWriter_test.cpp
      ${PROJECT_SOURCE_DIR}/test/umaa/LargeSetReader_test.cpp
      ${PROJECT_SOURCE_DIR}/test/umaa/UmaaUtils_test.cpp
      ${PROJECT_SOURCE_DIR}/test/umaa/conditionals/ConditionalBase_test.cpp
      ${PROJECT_SOURCE_DIR}/test/umaa/conditionals/ConditionalFactory_test.cpp
      ${PROJECT_SOURCE_DIR}/test/umaa/conditionals/ConstraintViolatedConditional_test.cpp
      ${PROJECT_SOURCE_DIR}/test/umaa/conditionals/DepthConditional_test.cpp
      ${PROJECT_SOURCE_DIR}/test/umaa/conditionals/DepthRateConditional_test.cpp
      ${PROJECT_SOURCE_DIR}/test/umaa/conditionals/HeadingSectorConditional_test.cpp
      ${PROJECT_SOURCE_DIR}/test/umaa/conditionals/LogicalANDConditional_test.cpp
      ${PROJECT_SOURCE_DIR}/test/umaa/conditionals/LogicalNOTConditional_test.cpp
      ${PROJECT_SOURCE_DIR}/test/umaa/conditionals/LogicalORConditional_test.cpp
      ${PROJECT_SOURCE_DIR}/test/umaa/conditionals/PitchRateConditional_test.cpp
      ${PROJECT_SOURCE_DIR}/test/umaa/conditionals/RelativeSpeedConditional_test.cpp
      ${PROJECT_SOURCE_DIR}/test/umaa/conditionals/RollRateConditional_test.cpp
      ${PROJECT_SOURCE_DIR}/test/umaa/conditionals/SpeedConditional_test.cpp
      ${PROJECT_SOURCE_DIR}/test/umaa/conditionals/TimeConditional_test.cpp
      ${PROJECT_SOURCE_DIR}/test/umaa/conditionals/WaterZoneConditional_test.cpp
      ${PROJECT_SOURCE_DIR}/test/umaa/conditionals/YawRateConditional_test.cpp
      ${PROJECT_SOURCE_DIR}/test/umaa/domain/AsyncCommandSessionTest.cpp
      ${PROJECT_SOURCE_DIR}/test/umaa/domain/UmaaCommandStatus_test.cpp
      ${PROJECT_SOURCE_DIR}/test/umaa/domain/UmaaCommandStatusBase_test.cpp
      ${PROJECT_SOURCE_DIR}/test/umaa/services/ActiveConstraintsControlProvider_test.cpp
      ${PROJECT_SOURCE_DIR}/test/umaa/services/ConditionalReportConsumer_test.cpp
      ${PROJECT_SOURCE_DIR}/test/umaa/services/ConditionalReportProvider_test.cpp
      ${PROJECT_SOURCE_DIR}/test/umaa/objective-executors/core/ObjectiveBaseTest.cpp
      ${PROJECT_SOURCE_DIR}/test/umaa/objective-executors/core/ObjectiveStateMachine_test.cpp
      ${PROJECT_SOURCE_DIR}/test/umaa/services/base/ReportConsumer_test.cpp
      ${PROJECT_SOURCE_DIR}/test/umaa/services/base/ReportProvider_test.cpp
      ${PROJECT_SOURCE_DIR}/test/umaa/services/base/CommandConsumerBase_test.cpp
      ${PROJECT_SOURCE_DIR}/test/umaa/services/base/CommandProviderBase_test.cpp
      ${PROJECT_SOURCE_DIR}/test/umaa/objective-executors/services/GlobalVectorControlServiceConsumerTest.cpp
      ${PROJECT_SOURCE_DIR}/test/umaa/objective-executors/services/GlobalWaypointControlServiceConsumerTest.cpp
      ${PROJECT_SOURCE_DIR}/test/umaa/objective-executors/services/ObjectiveExecutorControlServiceProviderTest.cpp
      ${PROJECT_SOURCE_DIR}/test/umaa/objective-executors/services/ObjectiveExecutorStateControlServiceProviderTest.cpp
      ${PROJECT_SOURCE_DIR}/test/umaa/objective-executors/ObjectiveControllerTest.cpp
      ${PROJECT_SOURCE_DIR}/test/umaa/specializations/factory/ObjectiveSpecFactory_test.cpp
      ${PROJECT_SOURCE_DIR}/test/umaa/specializations/data/SendableSpecialization_test.cpp
    )

    target_include_directories(umaa_cxx_test PUBLIC
      ${BASE_INCLUDE_DIRS}
      PRIVATE 
      ${GENERATED_IDL_TYPES}
      ${PROJECT_SOURCE_DIR}/test/observer
      ${PROJECT_SOURCE_DIR}/test/umaa/conditionals/util
      ${PROJECT_SOURCE_DIR}/test/io/dds/util
      ${PROJECT_SOURCE_DIR}/test/umaa/objective-executors/mocks
    )

    target_link_libraries(umaa_cxx_test
      ${TEST_LIBS}
      umaa-sdk-common
      umaa++
      umaa-cyclone-cxx-types
      CycloneDDS-CXX::ddscxx
    )
    target_compile_options(umaa_cxx_test PRIVATE ${COMPILE_OPTS})

    add_test(NAME umaa_cxx_test COMMAND umaa_cxx_test --gtest_output=xml:test-reports/umaa_cxx_test_report.xml)
  endif()

  #------------------------------#
  #        Cyclone Tests         #
  #------------------------------#
  if(TEST_CYCLONE)
  
    add_subdirectory(${PROJECT_SOURCE_DIR}/third-party/helloworld ${CMAKE_BINARY_DIR}/helloworld)
    file(COPY ${PROJECT_SOURCE_DIR}/test/io/dds/cyclone/TEST_QOS_PROFILES.xml DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
  
    add_executable(umaa_sdk_cyclone_test
      ${PROJECT_SOURCE_DIR}/test/io/dds/cyclone/CycloneBufferedReader_test.cpp
      ${PROJECT_SOURCE_DIR}/test/io/dds/cyclone/CycloneReader_test.cpp
      ${PROJECT_SOURCE_DIR}/test/io/dds/cyclone/CycloneSender_test.cpp
      ${PROJECT_SOURCE_DIR}/test/io/dds/cyclone/CycloneQosProviderWrapper_test.cpp
    )
    
    target_include_directories(umaa_sdk_cyclone_test PUBLIC
    ${BASE_INCLUDE_DIRS}
    ${CYCLONE_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}/helloworld
    )
    
    target_link_libraries(umaa_sdk_cyclone_test ${TEST_LIBS} umaa-sdk-common CycloneDDS-CXX::ddscxx umaa-cyclone-cxx-types hello-world-types)
    target_compile_options(umaa_sdk_cyclone_test PRIVATE ${COMPILE_OPTS})
  
    add_test(NAME umaa_sdk_cyclone_test COMMAND umaa_sdk_cyclone_test --gtest_output=xml:test-reports/umaa_sdk_cyclone_test_report.xml)
  endif()
endif()



