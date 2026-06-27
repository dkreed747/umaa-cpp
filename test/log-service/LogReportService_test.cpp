//---------------------------------------------------------------------------
// Copyright 2025 Pennsylvania State University
//
// Applied Research Laboratory
// Pennsylvania State University
// P.O. Box 30
// State College, PA 16804-0030
//
// DISTRIBUTION STATEMENT A. Approved for public release.
// Distribution is unlimited.
// This software was developed by the Department of the Navy,
// NAVSEA Unmanned and Small Combatants. It is provided under the terms of
// use found in the LICENSE file at the source code root directory.
//
//---------------------------------------------------------------------------

#include <string>

#include <dds/dds.hpp>
#include <gtest/gtest.h>

#include "CycloneUtilities.h"
#include "RealtimeSystemClock.h"
#include "UuidFactory.h"
#include "Logger.h"
#include <UMAA/SO/LogReport/LogReportType.hpp>

static const uint32_t SLEEP_MS = 2000;
static const int32_t testDomainId_ = 79;
static const std::string logTopicName_ = UMAA::SO::LogReport::LogReportTypeTopic;
static const std::string testSourceId_ = "00000000-0000-0000-0000-000000000123";

class LogReportServiceTest : public ::testing::Test {
 public:
  static void SetUpTestSuite() {
    const std::string testQos_ = "UMAA_QoS_Library::UMAA_Base_Profile";
    setenv("domain-qos-file", "CYCLONE_QOS_PROFILES.xml", true);
    setenv("domain-qos-profile", testQos_.c_str(), true);
    setenv("domain-id", std::to_string(testDomainId_).c_str(), true);
    setenv("SOURCE_ID", testSourceId_.c_str(), true);

    participant_ = arlcore::io::getDomainParticipant(testDomainId_);
    auto ddsTopic = arlcore::io::getTopic<UMAA::SO::LogReport::LogReportType>(participant_, logTopicName_);
    auto subscriber = arlcore::io::createSubscriber(participant_);
    logReportReader = dds::sub::DataReader<UMAA::SO::LogReport::LogReportType>(subscriber, ddsTopic, subscriber.default_datareader_qos());

  }

  void SetUp() {
    ASSERT_EQ(0U, logReportReader.take().length());
  }

  void TearDown() {

  }

  static dds::domain::DomainParticipant participant_;
  static dds::sub::DataReader<UMAA::SO::LogReport::LogReportType> logReportReader;
};

dds::domain::DomainParticipant LogReportServiceTest::participant_ = dds::core::null;
dds::sub::DataReader<UMAA::SO::LogReport::LogReportType> LogReportServiceTest::logReportReader = dds::core::null;

TEST_F(LogReportServiceTest, infoLogTest) {
  UMAA::SO::LogReport::LogReportType logReport;

  std::string testLogMsg = "Info test";

  UMAA_LOG_INFO(util::SYSTEM_LOGGER, testLogMsg);

  arlcore::RealtimeSystemClock clock;
  uint64_t send_time_sec;
  uint64_t send_time_nano;
  clock.getCurrentTime_umaa(&send_time_sec, &send_time_nano);

  const uint32_t MAX_CHECK_COUNT = 10;
  auto samples = logReportReader.take();
  
  for (uint32_t i = 0; i < MAX_CHECK_COUNT && samples.length() == 0; i++) {
    std::cout << "Waiting for messages..." << std::endl;
    samples = logReportReader.take();
    usleep(10000);
  }

  ASSERT_GT(samples.length(), 0);
  auto sampleIterator = samples.begin();
  const UMAA::SO::LogReport::LogReportType& actualLogData = sampleIterator->data();

  EXPECT_EQ(UMAA::Common::MaritimeEnumeration::LogLevelEnumModule::LogLevelEnumType::INFORMATION, actualLogData.level());
  EXPECT_EQ(testLogMsg, actualLogData.entry());

  arlcore::NumericGuid testSourceGuid = arlcore::UuidFactory::getInstance().parseGuidFromString(testSourceId_);
  EXPECT_EQ(testSourceGuid, actualLogData.source().id());

  EXPECT_EQ(send_time_sec, actualLogData.timeStamp().seconds());
  EXPECT_LT(send_time_nano - actualLogData.timeStamp().nanoseconds(), 5000000);
}

TEST_F(LogReportServiceTest, errorLogTest) {
  std::string testLogMsg = "Error test";

  UMAA_LOG_ERROR(util::SYSTEM_LOGGER, testLogMsg);

  arlcore::RealtimeSystemClock clock;
  uint64_t send_time_sec;
  uint64_t send_time_nano;
  clock.getCurrentTime_umaa(&send_time_sec, &send_time_nano);

  const uint32_t MAX_CHECK_COUNT = 10;
  auto samples = logReportReader.take();
  for (uint32_t i = 0; i < MAX_CHECK_COUNT && samples.length() == 0; i++) {
    std::cout << "Waiting for messages..." << std::endl;
    samples = logReportReader.take();
    usleep(10000);
  }

  ASSERT_GT(samples.length(), 0);
  auto sampleIterator = samples.begin();
  const UMAA::SO::LogReport::LogReportType& actualLogData = sampleIterator->data();

  EXPECT_EQ(UMAA::Common::MaritimeEnumeration::LogLevelEnumModule::LogLevelEnumType::ERROR, actualLogData.level());
  EXPECT_EQ(testLogMsg, actualLogData.entry());

  arlcore::NumericGuid testSourceGuid = arlcore::UuidFactory::getInstance().parseGuidFromString(testSourceId_);
  EXPECT_EQ(testSourceGuid, actualLogData.source().id());

  EXPECT_EQ(send_time_sec, actualLogData.timeStamp().seconds());
  EXPECT_LT(send_time_nano - actualLogData.timeStamp().nanoseconds(), 5000000);
}

TEST_F(LogReportServiceTest, warnLogTest) {
  std::string testLogMsg = "Warn test";

  UMAA_LOG_WARN(util::SYSTEM_LOGGER, testLogMsg);

  arlcore::RealtimeSystemClock clock;
  uint64_t send_time_sec;
  uint64_t send_time_nano;
  clock.getCurrentTime_umaa(&send_time_sec, &send_time_nano);

  const uint32_t MAX_CHECK_COUNT = 10;
  auto samples = logReportReader.take();
  for (uint32_t i = 0; i < MAX_CHECK_COUNT && samples.length() == 0; i++) {
    std::cout << "Waiting for messages..." << std::endl;
    samples = logReportReader.take();
    usleep(10000);
  }

  ASSERT_GT(samples.length(), 0);

  auto sampleIterator = samples.begin();
  const UMAA::SO::LogReport::LogReportType& actualLogData = sampleIterator->data();

  EXPECT_EQ(UMAA::Common::MaritimeEnumeration::LogLevelEnumModule::LogLevelEnumType::WARNING, actualLogData.level());
  EXPECT_EQ(testLogMsg, actualLogData.entry());

  arlcore::NumericGuid testSourceGuid = arlcore::UuidFactory::getInstance().parseGuidFromString(testSourceId_);
  EXPECT_EQ(testSourceGuid, actualLogData.source().id());

  EXPECT_EQ(send_time_sec, actualLogData.timeStamp().seconds());
  EXPECT_LT(send_time_nano - actualLogData.timeStamp().nanoseconds(), 5000000);
}