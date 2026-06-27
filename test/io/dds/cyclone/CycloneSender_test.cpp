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

#include "HelloWorldData.hpp"
#include "CycloneSender.h"


using arlcore::io::CycloneSender;
using arlcore::io::ReadStatus;
using arlcore::io::Duration;
using StringType = HelloWorldData::Msg;

class CycloneSenderTest : public ::testing::Test {
 protected:
  static void SetUpTestSuite() {
    // DDS Setup
    const int32_t domainId = 75;
    participant_ = arlcore::io::getDomainParticipant(domainId);

    // Topics
    const std::string topic = "StringTopic";
    const auto ddsTopic = arlcore::io::getTopic<StringType>(participant_, topic);

    // Create Subscriber and Publisher
    const auto ddsSubscriber = arlcore::io::createSubscriber(participant_);
    const auto ddsPublisher = arlcore::io::createPublisher(participant_);

    dds::sub::qos::DataReaderQos rQos;
    rQos << dds::core::policy::History::KeepAll()
         << dds::core::policy::Reliability::Reliable();
    dds::pub::qos::DataWriterQos wQos;
    wQos << dds::core::policy::History::KeepAll()
         << dds::core::policy::Reliability::Reliable();

    // Cyclone Sender
    cycloneSender_ = std::make_unique<CycloneSender<StringType>>(
      participant_, topic, wQos);

    // Test IO
    testReader_ = dds::sub::DataReader<StringType>(
      ddsSubscriber, ddsTopic, rQos);

    usleep(10000);  // Discovery 0.01 seconds
  }
  void SetUp() override {
    testReader_.take();
  }

  static dds::domain::DomainParticipant participant_;
  static dds::sub::DataReader<StringType> testReader_;
  static std::unique_ptr<CycloneSender<StringType>> cycloneSender_;
  static const Duration maxWait_;
};

dds::domain::DomainParticipant CycloneSenderTest::participant_ = dds::core::null;
dds::sub::DataReader<StringType> CycloneSenderTest::testReader_ = dds::core::null;
std::unique_ptr<CycloneSender<StringType>> CycloneSenderTest::cycloneSender_;
const Duration CycloneSenderTest::maxWait_ = {5, 0};


TEST_F(CycloneSenderTest, Send) {
  StringType testSamples[5];
  StringType sample1(1, "Red1");  // Test same instance
  StringType sample2(1, "Red2");
  StringType sample3(1, "Red3");
  StringType sample4(2, "Blue"); // Test different instances
  StringType sample5(3, "Green");
  testSamples[0] = sample1;
  testSamples[1] = sample2;
  testSamples[2] = sample3;
  testSamples[3] = sample4;
  testSamples[4] = sample5;

  for (auto sample : testSamples) {
    cycloneSender_->send(sample);
  }
   usleep(50000); 

  auto readSamples = testReader_.take();
  EXPECT_EQ(readSamples.length(), 5);

  auto testCount = 0;
  for (const auto& sample : readSamples) {
    ASSERT_EQ(sample.data().userID(), testSamples[testCount].userID());
    ASSERT_EQ(sample.data().message(), testSamples[testCount].message());
    testCount++;
  }
}

TEST_F(CycloneSenderTest, Dispose) {
  StringType sample(1, "String sample to dispose");
  cycloneSender_->send(sample);

  auto readSamples = testReader_.take();
  EXPECT_EQ(readSamples.length(), 1);

  cycloneSender_->dispose(sample);

  readSamples = testReader_.take();
  EXPECT_EQ(readSamples.length(), 1);
  auto firstSample  = *readSamples.begin();
  ASSERT_EQ(firstSample.info().state().instance_state(), dds::sub::status::InstanceState::not_alive_disposed());
}