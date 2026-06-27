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
#include <stdlib.h>

#include "HelloWorldData.hpp"
#include "CycloneReader.h"

using arlcore::io::CycloneReader;
using arlcore::io::ReadStatus;
using StringType = HelloWorldData::Msg;

class CycloneReaderTest : public ::testing::Test {
 protected:
  const int32_t DOMAIN_ID = rand() % 100;
  const std::string TOPIC = "StringTopic";

  void SetUpDDS() {
    participant_ = arlcore::io::getDomainParticipant(DOMAIN_ID);
    // Create Topics
    const auto ddsTopic = arlcore::io::getTopic<StringType>(participant_, TOPIC);
    // Create Subscriber and Publisher
    const auto ddsSubscriber = arlcore::io::createSubscriber(participant_);
    const auto ddsPublisher = arlcore::io::createPublisher(participant_);
    // Create Reader
    reader_ = std::make_shared<CycloneReader<StringType>>(
      participant_, TOPIC, ddsSubscriber.default_datareader_qos());
    // Test IO
    testWriter_ = dds::pub::DataWriter<StringType>(
      ddsPublisher, ddsTopic, ddsPublisher.default_datawriter_qos());
  }

  void SetUp() override {
    SetUpDDS();
    // Test Samples
    StringType sample1(1, "Red"); 
    StringType sample2(2, "Blue");
    StringType sample3(3, "Green");
    StringType sample4(4, "Red");
    StringType sample5(5, "Red");
    testSamples_[0] = sample1;
    testSamples_[1] = sample2;
    testSamples_[2] = sample3;
    testSamples_[3] = sample4;
    testSamples_[4] = sample5;


    usleep(10000);  // Discovery 0.01 seconds
    StringType temp;
    ASSERT_EQ(reader_->readLatest(&temp), ReadStatus::NO_DATA);
    for (auto sample : testSamples_) {
      testWriter_.write(sample);
    }
  }

  dds::domain::DomainParticipant participant_= dds::core::null;
  dds::pub::DataWriter<StringType> testWriter_ = dds::core::null;
  std::shared_ptr<CycloneReader<StringType>> reader_;
  const dds::core::Duration waitDuration_ = dds::core::Duration(5,0);
  StringType testSamples_[5];
};

class TestFilter : public arlcore::io::ReaderFilter<StringType> {
  bool filter(const StringType& sample) override {
    return sample.message() == "Red";
  }
};

TEST_F(CycloneReaderTest, Read) {
  StringType data;
  for (auto sample : testSamples_) {
    ASSERT_EQ(reader_->read(&data), ReadStatus::SUCCESS);
    EXPECT_EQ(data, sample);
  }

  ASSERT_EQ(reader_->read(&data), ReadStatus::NO_DATA);

  // Check health only on first one since we are reusing the reader
  auto health = reader_->getHealthInfo();
  EXPECT_EQ(health.matchedSenders, 1);
  EXPECT_EQ(health.rejectedSamples, 0);
  EXPECT_EQ(health.lostSamples, 0);
  EXPECT_EQ(health.incompatibleQos, 0);
  EXPECT_EQ(health.invalidSamples, 0);
  EXPECT_EQ(health.readSamples, 5);
}

TEST_F(CycloneReaderTest, readLatest) {
  StringType data;

  ASSERT_EQ(reader_->readLatest(&data), ReadStatus::SUCCESS);
  EXPECT_EQ(data, testSamples_[4]);  // Even though we sent 5, readLatest should only return the freshest sample

  ASSERT_EQ(reader_->readLatest(&data), ReadStatus::NO_DATA);
}

TEST_F(CycloneReaderTest, readUpToN) {
  arlcore::io::SampleEnvelope<StringType> dataPackage[5];

  // read up to two
  ASSERT_EQ(reader_->readUpToN(dataPackage, 2), 2);

  // try to read another 8 (note we only sent 5 so there should be 3 left)
  ASSERT_EQ(reader_->readUpToN(&dataPackage[2], 8), 3);

  for (uint16_t i = 0; i < 5; i++) {
    EXPECT_EQ(dataPackage[i].data, testSamples_[i]);
  }
}

TEST_F(CycloneReaderTest, ReadDispose) {
  StringType data;
  ASSERT_EQ(reader_->readLatest(&data), ReadStatus::SUCCESS);
  EXPECT_EQ(data, testSamples_[4]);

  testWriter_.dispose_instance(testWriter_.lookup_instance(testSamples_[0]));

  ASSERT_EQ(reader_->read(&data), ReadStatus::DISPOSED);
  EXPECT_EQ(data.userID(), testSamples_[0].userID());
}

TEST_F(CycloneReaderTest, ReadFiltered) {
  arlcore::io::SampleEnvelope<StringType> dataPackage[5];
  auto filter = std::make_shared<TestFilter>();
  reader_->setManualFilter(filter);

  ASSERT_EQ(reader_->readUpToN(dataPackage, 5), 3);
  EXPECT_EQ(dataPackage[0].data, testSamples_[0]);
  EXPECT_EQ(dataPackage[1].data, testSamples_[3]);
  EXPECT_EQ(dataPackage[2].data, testSamples_[4]);
}

TEST_F(CycloneReaderTest, ReadLatestFiltered) {
  StringType sample6(6, "Green");
  testWriter_.write(sample6);
  auto filter = std::make_shared<TestFilter>();
  reader_->setManualFilter(filter);

  StringType data;
  ASSERT_EQ(reader_->readLatest(&data), ReadStatus::SUCCESS);
  EXPECT_EQ(data, testSamples_[4]);  // readLatest should return the latest sample that matches the filter

  ASSERT_EQ(reader_->readLatest(&data), ReadStatus::NO_DATA);
}

TEST_F(CycloneReaderTest, readInstance) {
  StringType data;
  StringType data1;
  StringType data2;
  StringType key(2, ""); // 2, Blue
  StringType key2(3,""); // 3, Green
  // Read once and get data
  ASSERT_EQ(reader_->readInstance(key, &data), ReadStatus::SUCCESS);
  // Read the same key a second time and get no data (because we used take)
  ASSERT_EQ(reader_->readInstance(key, &data1), ReadStatus::NO_DATA);
  // Read another key and get data
  reader_->readInstance(key2, &data2);
  EXPECT_EQ(data,testSamples_[1]);
  EXPECT_EQ(data2,testSamples_[2]);
  arlcore::io::SampleEnvelope<StringType> envelope[10];
  // After reading two keyed samples, there should be 3 samples (out of 5) left on the bus
  EXPECT_EQ(reader_->readUpToN(envelope, 10), 3);
  EXPECT_EQ(reader_->read(&data1), ReadStatus::NO_DATA);
}

TEST_F(CycloneReaderTest, readDisposedInstance) {
  StringType data;
  StringType key(2, ""); // 2, Blue
  // Read once and get data
  reader_->readInstance(key, &data);
  EXPECT_EQ(data, testSamples_[1]);

  testWriter_.dispose_instance(testWriter_.lookup_instance(testSamples_[1]));
  ASSERT_EQ(reader_->readInstance(key, &data), ReadStatus::DISPOSED);
  // Data is set to just the key
  EXPECT_EQ(data.userID(), testSamples_[1].userID());
  EXPECT_EQ(data.message(), "");
}

TEST_F(CycloneReaderTest, readFilteredInstance) {
  auto filter = std::make_shared<TestFilter>(); // Filters on "Red"
  reader_->setManualFilter(filter);
  StringType data;
  StringType key(2, ""); // 2, Blue
  StringType key1(4, ""); // 4, Red

  // Read once and get data
  EXPECT_EQ(reader_->readInstance(key, &data), ReadStatus::NO_DATA);

  ASSERT_EQ(reader_->readInstance(key1, &data), ReadStatus::SUCCESS);
  EXPECT_EQ(data.userID(), testSamples_[3].userID());
  EXPECT_EQ(data.message(), "Red");
}