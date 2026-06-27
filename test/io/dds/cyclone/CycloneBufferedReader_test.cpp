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
#include <chrono>

#include "HelloWorldData.hpp"
#include "CycloneBufferedReader.h"
#include "CycloneUtilities.h"

using arlcore::io::SampleEnvelope;
using arlcore::io::CycloneBufferedReader;
using arlcore::io::ReadStatus;
using DdsType = HelloWorldData::Msg;

class CycloneBufferedReaderTest : public ::testing::Test {
 protected:
  static void SetUpTestSuite() {

    // DDS Setup
    const int32_t domainId = 78;
    participant_ = arlcore::io::getDomainParticipant(domainId);

    // Topics
    const std::string topic = "StringTopic";
    const auto ddsTopic = arlcore::io::getTopic<DdsType>(participant_, topic);

    // Publisher and Subscriber
    const auto ddsPublisher = arlcore::io::createPublisher(participant_);
    const auto ddsSubcriber = arlcore::io::createSubscriber(participant_);

    // Cyclone Buffered Reader
    bufferedReader_ = std::make_unique<CycloneBufferedReader<DdsType>>(
      participant_, topic, ddsSubcriber.default_datareader_qos());

    ASSERT_TRUE(bufferedReader_->startReaderThread());

    // Test IO
    testWriter_ = dds::pub::DataWriter<DdsType>(
      ddsPublisher, ddsTopic, ddsPublisher.default_datawriter_qos());

    usleep(10000);  // Discovery 0.01 seconds
  }

  static void TearDownTestSuite() {
    // Code to run at the end of the test suite
    ASSERT_TRUE(bufferedReader_->stopReaderThread());
  }

  const std::chrono::milliseconds SLEEP_TIMER = std::chrono::milliseconds(2000);
  static dds::domain::DomainParticipant participant_;
  static dds::pub::DataWriter<DdsType> testWriter_;
  static std::unique_ptr<CycloneBufferedReader<DdsType>> bufferedReader_;
};

dds::domain::DomainParticipant CycloneBufferedReaderTest::participant_ = dds::core::null;
dds::pub::DataWriter<DdsType> CycloneBufferedReaderTest::testWriter_ = dds::core::null;
std::unique_ptr<CycloneBufferedReader<DdsType>> CycloneBufferedReaderTest::bufferedReader_;

TEST_F(CycloneBufferedReaderTest, readSample) {
  const DdsType sample(1, "world");
  testWriter_->write(sample);
  dds::core::Duration wait(10, 0);
  std::this_thread::sleep_for(SLEEP_TIMER);

  EXPECT_EQ(bufferedReader_->size(), 1);
  SampleEnvelope<DdsType> read_sample;
  EXPECT_EQ(bufferedReader_->read(&read_sample.data), ReadStatus::SUCCESS);
  EXPECT_EQ(read_sample.data, sample);
}

TEST_F(CycloneBufferedReaderTest, readDisposedSample) {
  const DdsType sample(2, "data");
  testWriter_->write(sample);
  std::this_thread::sleep_for(SLEEP_TIMER);

  ASSERT_EQ(bufferedReader_->size(), 1);
  SampleEnvelope<DdsType> recieved_sample;
  EXPECT_EQ(bufferedReader_->read(&recieved_sample.data), ReadStatus::SUCCESS);
  EXPECT_EQ(recieved_sample.data, sample);

  dds::core::InstanceHandle instanceHandle = testWriter_->lookup_instance(sample);
  ASSERT_FALSE(instanceHandle.is_nil());
  testWriter_.dispose_instance(instanceHandle);
  testWriter_.dispose_instance(instanceHandle);
  std::this_thread::sleep_for(SLEEP_TIMER);

  EXPECT_EQ(bufferedReader_->size(), 1);
  EXPECT_EQ(bufferedReader_->read(&recieved_sample.data), ReadStatus::DISPOSED);
  EXPECT_EQ(recieved_sample.data.userID(), sample.userID());
}