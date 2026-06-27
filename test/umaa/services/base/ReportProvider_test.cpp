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

#include <dds/dds.hpp>

#include <UMAA/SA/GlobalPoseStatus/GlobalPoseReportType.hpp>

#include "Logger.h"
#include "ReportProvider.h"
#include "CycloneSender.h"
#include "CycloneUtilities.h"
#include "NumericGuid.h"

using GpType = UMAA::SA::GlobalPoseStatus::GlobalPoseReportType;
using DateType = UMAA::Common::Measurement::DateTime;
using arlcore::io::CycloneSender;
using arlcore::umaa::services::ReportProvider;
using arlcore::NumericGuid;
using arlcore::io::SendStatus;

class ReportProviderTest : public ::testing::Test {
 protected:
  static void SetUpTestSuite() {
    // DDS Setup
    const int32_t domainId = 84;
    participant_ = arlcore::io::getDomainParticipant(domainId);

    // Topics
    const std::string topic = UMAA::SA::GlobalPoseStatus::GlobalPoseReportTypeTopic;
    const auto ddsTopic = arlcore::io::getTopic<GpType>(participant_, topic);

    // Publishers and Subscribers
    const auto ddsPub = arlcore::io::createPublisher(participant_);
    const auto ddsSub = arlcore::io::createSubscriber(participant_);

    // QoS Settings
    dds::sub::qos::DataReaderQos rQos;
    rQos << dds::core::policy::History::KeepAll();

    dds::pub::qos::DataWriterQos wQos;
    wQos << dds::core::policy::History::KeepAll();

    // Report Provider
    providerWriter_ = std::make_shared<CycloneSender<GpType>>(
        participant_, topic, wQos);

    reportProvider_ = std::make_unique<ReportProvider<GpType>>(
      reportSourceId_, providerWriter_);

    // Test IO
    testReader_ = dds::sub::DataReader<GpType>(
      ddsSub, ddsTopic, rQos);

    usleep(10000);  // Discovery 0.01 seconds
  }

  void SetUp() override {
    // Clear test reader queue prior to each test case
    testReader_.take();
  }

  // Helper function to test the dispose action in the destructor when this function goes out of scope
  static void disposeOnDeath() {
    ReportProvider<GpType> localReportProvider(reportSourceId_, providerWriter_);
    GpType sample;
    usleep(10000); // Discovery of new report provider
    localReportProvider.send(&sample);
    usleep(1000);  // IO 0.001 seconds
    auto sampleLoan = testReader_.take();
    ASSERT_EQ(sampleLoan.length(), 1);
  }

  static dds::domain::DomainParticipant participant_;
  static const NumericGuid reportSourceId_;
  static dds::sub::DataReader<GpType> testReader_;
  static std::shared_ptr<CycloneSender<GpType>> providerWriter_;
  static std::unique_ptr<ReportProvider<GpType>> reportProvider_;
};

dds::domain::DomainParticipant ReportProviderTest::participant_ = dds::core::null;
const NumericGuid ReportProviderTest::reportSourceId_(
  {77, 105, 99, 104, 101, 108, 108, 101, 0, 105, 115, 0, 99, 111, 111, 108});
dds::sub::DataReader<GpType> ReportProviderTest::testReader_ = dds::core::null;
std::shared_ptr<CycloneSender<GpType>> ReportProviderTest::providerWriter_;
std::unique_ptr<ReportProvider<GpType>> ReportProviderTest::reportProvider_;

TEST_F(ReportProviderTest, SendReports) {
  const uint16_t kNumReports = 10;
  GpType reports[kNumReports];

  // Send `kNumReports` reports with unique depth values
  for (uint16_t i = 0; i < kNumReports; i++) {
    reports[i].course(i);
    reportProvider_->send(&reports[i]);
  }

  usleep(10000);  // IO 0.01 seconds

  auto samples = testReader_.take();

  for (const auto & sample:samples) {
    if (!sample.info().valid())
      continue;
    const auto &data = sample.data();
  }
  ASSERT_EQ(samples.length(), 10);
  
  // Test that the received reports have had their source ID fields filled out by the provider
  int i = 0;
  for (auto sample = samples.begin(); sample < samples.end(); ++sample, ++i) {
    EXPECT_EQ(reportSourceId_.getGuid(), sample->data().source().id());
    EXPECT_EQ(reports[i].course(), sample->data().course());
  }
}

TEST_F(ReportProviderTest, DisposeReportsOnDestruction) {
  disposeOnDeath();
  usleep(1000);  // IO 0.001 seconds

  auto sampleLoan = testReader_.take();
  ASSERT_EQ(sampleLoan.length(), 1);
  auto sample = sampleLoan.begin();

  GpType keySample;
  testReader_.key_value(keySample, sample->info().instance_handle());

  EXPECT_EQ(sample->info().state().instance_state(), dds::sub::status::InstanceState::not_alive_disposed());
  EXPECT_EQ(reportSourceId_.getGuid(), keySample.source().id());
}
