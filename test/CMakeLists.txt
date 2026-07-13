#---------------------------------------------------------------------------
# Applied Research Laboratory
# Pennsylvania State University
# P.O. Box 30
# State College, PA 16804-0030
#
# Copyright 2022 The Pennsylvania State University
#---------------------------------------------------------------------------

find_package(GTest CONFIG REQUIRED)

configure_file(${PROJECT_SOURCE_DIR}/config/CYCLONE_QOS_PROFILES.xml ${CMAKE_CURRENT_BINARY_DIR}/CYCLONE_QOS_PROFILES.xml COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/config/log4cxx.xml ${CMAKE_CURRENT_BINARY_DIR}/log4cxx.xml COPYONLY)

#------------------------------#
#    Core (non-UMAA) tests     #
#------------------------------#
if(UMAA_CPP_BUILD_TESTS)

  add_executable(umaa-cpp-core-test
    domain/GlobalPoseData_test.cpp
    domain/SpeedStatusData_test.cpp
    domain/BatteryMgmtStatusData_test.cpp
    observer/InputConnector_test.cpp
    common/RealtimeSystemClock_test.cpp
    common/UuidBase_test.cpp
    io/udp/NetworkUtilities_test.cpp
    io/dds/base/BufferedReaderBase_test.cpp
    io/dds/IOReaderRegistryTest.cpp
    io/dds/IOWriterRegistryTest.cpp
    env/Env_Test.cpp)

  target_link_libraries(umaa-cpp-core-test PRIVATE
    umaa-cpp::umaa-cpp
    umaa-cpp::test-utils
    GTest::gtest_main)

  add_test(NAME umaa-cpp-core-test
    COMMAND umaa-cpp-core-test --gtest_output=xml:test-reports/umaa-cpp-core-test.xml)

  #------------------------------#
  #   UMAA implementation tests  #
  #------------------------------#
  add_executable(umaa-cpp-umaa-test
    observer/Subject_test.cpp
    config/ConfigurationManager_test.cpp
    log-service/LogReportService_test.cpp
    health-status/HealthStatusControl_test.cpp
    io/dds/base/ObservableReader_test.cpp
    io/dds/util/LocalReaderSender_test.cpp
    algorithms/GeographicUtils_test.cpp
    algorithms/VectorUtils_test.cpp
    umaa/CommandStateMachine_test.cpp
    umaa/TaskStateMachine_test.cpp
    umaa/DestinationReaderFilter_test.cpp
    umaa/LargeList_test.cpp
    umaa/LargeListWriter_test.cpp
    umaa/LargeListReader_test.cpp
    umaa/LargeSet_test.cpp
    umaa/LargeSetWriter_test.cpp
    umaa/LargeSetReader_test.cpp
    umaa/UmaaUtils_test.cpp
    umaa/conditionals/ConditionalBase_test.cpp
    umaa/conditionals/ConditionalFactory_test.cpp
    umaa/conditionals/ConstraintViolatedConditional_test.cpp
    umaa/conditionals/DepthConditional_test.cpp
    umaa/conditionals/DepthRateConditional_test.cpp
    umaa/conditionals/HeadingSectorConditional_test.cpp
    umaa/conditionals/LogicalANDConditional_test.cpp
    umaa/conditionals/LogicalNOTConditional_test.cpp
    umaa/conditionals/LogicalORConditional_test.cpp
    umaa/conditionals/PitchRateConditional_test.cpp
    umaa/conditionals/RelativeSpeedConditional_test.cpp
    umaa/conditionals/RollRateConditional_test.cpp
    umaa/conditionals/SpeedConditional_test.cpp
    umaa/conditionals/TimeConditional_test.cpp
    umaa/conditionals/WaterZoneConditional_test.cpp
    umaa/conditionals/YawRateConditional_test.cpp
    umaa/domain/AsyncCommandSessionTest.cpp
    umaa/domain/UmaaCommandStatus_test.cpp
    umaa/domain/UmaaCommandStatusBase_test.cpp
    umaa/services/ActiveConstraintsControlProvider_test.cpp
    umaa/services/ConditionalReportConsumer_test.cpp
    umaa/services/ConditionalReportProvider_test.cpp
    umaa/objective-executors/core/ObjectiveBaseTest.cpp
    umaa/objective-executors/core/ObjectiveStateMachine_test.cpp
    umaa/services/base/ReportConsumer_test.cpp
    umaa/services/base/ReportProvider_test.cpp
    umaa/services/base/CommandConsumerBase_test.cpp
    umaa/services/base/CommandProviderBase_test.cpp
    umaa/objective-executors/services/GlobalVectorControlServiceConsumerTest.cpp
    umaa/objective-executors/services/GlobalWaypointControlServiceConsumerTest.cpp
    umaa/objective-executors/services/ObjectiveExecutorControlServiceProviderTest.cpp
    umaa/objective-executors/services/ObjectiveExecutorStateControlServiceProviderTest.cpp
    umaa/objective-executors/ObjectiveControllerTest.cpp
    umaa/specializations/factory/ObjectiveSpecFactory_test.cpp
    umaa/specializations/data/SendableSpecialization_test.cpp
  )

  target_include_directories(umaa-cpp-umaa-test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/observer
    ${CMAKE_CURRENT_SOURCE_DIR}/umaa/conditionals/util
    ${CMAKE_CURRENT_SOURCE_DIR}/umaa/objective-executors/mocks)

  target_link_libraries(umaa-cpp-umaa-test PRIVATE
    umaa-cpp::umaa-cpp
    umaa-cpp::test-utils
    GTest::gtest_main)

  add_test(NAME umaa-cpp-umaa-test
    COMMAND umaa-cpp-umaa-test --gtest_output=xml:test-reports/umaa-cpp-umaa-test.xml)
endif()

#------------------------------#
#  Cyclone DDS integration     #
#------------------------------#
if(UMAA_CPP_BUILD_DDS_TESTS)

  add_subdirectory(${PROJECT_SOURCE_DIR}/third-party/helloworld ${CMAKE_BINARY_DIR}/helloworld)
  file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/io/dds/cyclone/TEST_QOS_PROFILES.xml DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

  add_executable(umaa-cpp-dds-test
    io/dds/cyclone/CycloneBufferedReader_test.cpp
    io/dds/cyclone/CycloneReader_test.cpp
    io/dds/cyclone/CycloneSender_test.cpp
    io/dds/cyclone/CycloneQosProviderWrapper_test.cpp
  )

  target_link_libraries(umaa-cpp-dds-test PRIVATE
    umaa-cpp::umaa-cpp
    umaa-cpp::test-utils
    hello-world-types
    GTest::gtest_main)

  add_test(NAME umaa-cpp-dds-test
    COMMAND umaa-cpp-dds-test --gtest_output=xml:test-reports/umaa-cpp-dds-test.xml)

  # Run over loopback only so the test passes on CI runners and dev machines
  # without multicast-capable networks.
  set_tests_properties(umaa-cpp-dds-test PROPERTIES
    ENVIRONMENT "CYCLONEDDS_URI=file://${PROJECT_SOURCE_DIR}/config/cyclonedds-loopback.xml")
endif()
