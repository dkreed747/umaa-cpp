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

#include <gtest/gtest.h>

#include "chrono"
#include "memory"
#include "thread"

#include <dds/dds.hpp>

#include <UMAA/SA/GlobalPoseStatus/GlobalPoseReportType.hpp>
#include <UMAA/Common/Measurement/Measurements.hpp>

#include "Logger.h"
#include "ReportConsumer.h"
#include "CycloneReader.h"
#include "NumericGuid.h"
#include "TestObserver.h"


using GpType = UMAA::SA::GlobalPoseStatus::GlobalPoseReportType;
using DateType = UMAA::Common::Measurement::DateTime;
using arlcore::io::CycloneReader;
using arlcore::umaa::services::ReportConsumer;
using arlcore::NumericGuid;
using arlcore::io::ReadStatus;

class ReportConsumerTest : public ::testing::Test {
 protected:
  static void SetUpTestSuite() {
    // DDS Setup
    const int32_t domainId = 85;
    participant_ = arlcore::io::getDomainParticipant(domainId);

    // Publisher and Subscriber
    subscriber_ = arlcore::io::createSubscriber(participant_);
    publisher_ = arlcore::io::createPublisher(participant_);

    // Topics
    const std::string topic = UMAA::SA::GlobalPoseStatus::GlobalPoseReportTypeTopic;
    const auto ddsTopic = arlcore::io::getTopic<GpType>(participant_, topic);
    const dds::topic::Filter commandFilter("source.id = &hex(" + reportSourceId_.getTwoDigitHexString() + ")");

    // Report Consumer
    reportConsumer_ = std::make_unique<ReportConsumer<GpType>>(
      std::make_shared<CycloneReader<GpType>>(
        participant_, topic, subscriber_.default_datareader_qos()));


    // Test IO
    testWriter_ = dds::pub::DataWriter<GpType>(
      publisher_, ddsTopic, publisher_.default_datawriter_qos());

    observer_ = std::make_shared<arlcoretest::TestObserver<GpType>>();

    reportConsumer_->getReportSubject().registerObserver(observer_);

    usleep(10000);  // Discovery 0.01 seconds
  }

  void TearDown() override {
    observer_->clear();
  }

  static dds::domain::DomainParticipant participant_;
  static dds::sub::Subscriber subscriber_;
  static dds::pub::Publisher publisher_;
  static dds::core::detail::QosProvider qosProvider_;
  static dds::pub::DataWriter<GpType> testWriter_;
  static const NumericGuid reportSourceId_;
  static std::unique_ptr<ReportConsumer<GpType>> reportConsumer_;
  static std::shared_ptr<arlcoretest::TestObserver<GpType>> observer_;
};

dds::domain::DomainParticipant ReportConsumerTest::participant_ = dds::core::null;
dds::sub::Subscriber ReportConsumerTest::subscriber_ = dds::core::null;
dds::pub::Publisher ReportConsumerTest::publisher_ = dds::core::null;
dds::pub::DataWriter<GpType> ReportConsumerTest::testWriter_ = dds::core::null;
const NumericGuid ReportConsumerTest::reportSourceId_(
  {68, 101, 118, 111, 110, 32, 105, 115, 32, 99, 111, 111, 108, 0, 0, 0});
std::unique_ptr<ReportConsumer<GpType>> ReportConsumerTest::reportConsumer_;
std::shared_ptr<arlcoretest::TestObserver<GpType>> ReportConsumerTest::observer_;

TEST_F(ReportConsumerTest, ReadReports) {
  const uint16_t kNumReports = 10;
  GpType reports[kNumReports];
  for (uint32_t i = 0; i < kNumReports; i++) {
    DateType date(i, i);
    reports[i].source().id(reportSourceId_.getGuid());
    reports[i].timeStamp(date);
  }

  // Test Consecutive (first 5 reports)
  // This covers the case for when the User is calling read() faster than the provider is writing
  // Write -> Read -> Write -> Read -> Write -> Read -> Write -> Read -> Write -> Read
  for (uint32_t i = 0; i < 5; i++) {
    GpType recvGpType;
    testWriter_.write(reports[i]);
    usleep(1000);  // IO 0.001 seconds
    ASSERT_EQ(reportConsumer_->cycle(), ReadStatus::SUCCESS);
    recvGpType = reportConsumer_->getReport().value();
    EXPECT_EQ(reports[i].timeStamp(), recvGpType.timeStamp());
    EXPECT_EQ(observer_->getDataCount(), 1);
    EXPECT_EQ(recvGpType, observer_->getNextData());
  }

  // Test Latest (last report received after remaining 5 are sent)
  // This covers the case for when the User is calling read() slower than the provider is writing
  // Write -> Write -> Write -> Write -> Write -> Read(should be latest on bus)
  for (uint32_t i = 5; i < kNumReports; i++) {
    testWriter_.write(reports[i]);
  }

  usleep(1000);  // IO 0.001 seconds
  GpType recvGpType;
  ASSERT_EQ(reportConsumer_->cycle(), ReadStatus::SUCCESS);
  recvGpType = reportConsumer_->getReport().value();
  EXPECT_EQ(reports[kNumReports - 1].timeStamp(), recvGpType.timeStamp());
  EXPECT_EQ(observer_->getDataCount(), 1);
  EXPECT_EQ(recvGpType, observer_->getNextData());
}
