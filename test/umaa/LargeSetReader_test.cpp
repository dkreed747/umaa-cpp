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

#include "LargeSetReader.h"

#include <gtest/gtest.h>

#include <UMAA/MM/BaseType/TaskPlanType.hpp>
#include <dds/dds.hpp>

#include "CycloneReader.h"
#include "CycloneSender.h"
#include "UuidFactory.h"

using ObjectiveSetElement = UMAA::MM::BaseType::TaskPlanTypeObjectivesSetElement;
using UMAA::MM::BaseType::ObjectiveType;
using UMAA::Common::LargeSetMetadata;
using UMAA::Common::Measurement::DateTime;
using UMAA::MM::BaseType::TaskPlanTypeObjectivesSetElementTopic;
using UMAA::MM::BaseType::TaskPlanType;

class LargeSetReaderTest : public ::testing::Test {
 protected:
  static void SetUpTestSuite() {
    participant_ = arlcore::io::getDomainParticipant(domainId_);
    const auto subscriber_ = arlcore::io::createSubscriber(participant_);
    const auto publisher_ = arlcore::io::createPublisher(participant_);
    reader_ = std::make_shared<arlcore::io::CycloneReader<ObjectiveSetElement>>(
        participant_, setTopic_, subscriber_.default_datareader_qos());
    sender_ = std::make_shared<arlcore::io::CycloneSender<ObjectiveSetElement>>(
        participant_, setTopic_, publisher_.default_datawriter_qos());
    objectives_[0] = initializeObjective("obj_1");
    objectives_[1] = initializeObjective("obj_2");
    objectives_[2] = initializeObjective("obj_3");
    objectives_[3] = initializeObjective("obj_4");
    objectives_[4] = initializeObjective("obj_5");

    usleep(30000);  // Discovery action
  }
  void SetUp() override {
    setReader_ = std::make_unique<arlcore::umaa::LargeSetReader<ObjectiveType, ObjectiveSetElement>>(reader_);
  }

  static dds::domain::DomainParticipant participant_;
  static const int32_t domainId_;
  static const std::string setTopic_;
  static dds::sub::Subscriber subscriber_;
  static dds::pub::Publisher publisher_;
  static std::shared_ptr<arlcore::io::ReaderBase<ObjectiveSetElement>> reader_;
  static std::shared_ptr<arlcore::io::SenderBase<ObjectiveSetElement>> sender_;
  std::unique_ptr<arlcore::umaa::LargeSetReader<ObjectiveType, ObjectiveSetElement>> setReader_;
  static const arlcore::io::Duration maxWait_;
  static ObjectiveType objectives_[5];

  static ObjectiveSetElement initializeSetElement(LargeSetMetadata& metadata, const ObjectiveType& element) {
    static uint64_t timestamp_sec = 0;
    ObjectiveSetElement e;
    e.setID(metadata.setID());
    e.elementID(arlcore::UuidFactory::getInstance().generateGuid().getGuid());
    e.element(element);
    e.elementTimestamp(DateTime(timestamp_sec, 0));
    metadata.size(metadata.size() + 1);
    metadata.updateElementID(e.elementID());
    metadata.updateElementTimestamp(e.elementTimestamp());
    timestamp_sec++;
    return e;
  }

  static ObjectiveType initializeObjective(std::string name) {
    ObjectiveType obj;
    obj.name(name);
    obj.objectiveID(arlcore::UuidFactory::getInstance().generateGuid().getGuid());
    return obj;
  }

  LargeSetMetadata initializeMetadata() {
    LargeSetMetadata metadata;
    metadata.setID(arlcore::UuidFactory::getInstance().generateGuid().getGuid());
    metadata.updateElementID(arlcore::NIL_GUID.getGuid());
    metadata.size(0);
    return metadata;
  }
};

dds::domain::DomainParticipant LargeSetReaderTest::participant_ = dds::core::null;
const int32_t LargeSetReaderTest::domainId_ = 81;
const std::string LargeSetReaderTest::setTopic_ = TaskPlanTypeObjectivesSetElementTopic;
std::shared_ptr<arlcore::io::ReaderBase<ObjectiveSetElement>> LargeSetReaderTest::reader_(nullptr);
std::shared_ptr<arlcore::io::SenderBase<ObjectiveSetElement>> LargeSetReaderTest::sender_(nullptr);
const arlcore::io::Duration LargeSetReaderTest::maxWait_ = {0, 1000000};  // 0.001 seconds
ObjectiveType LargeSetReaderTest::objectives_[5];

TEST_F(LargeSetReaderTest, emptySet) {
  auto metadata = initializeMetadata();
  EXPECT_EQ(setReader_->getSetFromMetadata(metadata).status, arlcore::umaa::LargeSetStatus::EMPTY_SET);
}

TEST_F(LargeSetReaderTest, disposeSet) {
  auto metadata = initializeMetadata();
  ObjectiveSetElement e = initializeSetElement(metadata, objectives_[0]);
  EXPECT_EQ(sender_->send(e), arlcore::io::SendStatus::SUCCESS);
  sender_->waitForAcknowledgements(maxWait_);

  auto result = setReader_->getSetFromMetadata(metadata);
  EXPECT_EQ(result.status, arlcore::umaa::LargeSetStatus::VALID_SET);
  EXPECT_FALSE(result.set.expired());
  if (auto set = result.set.lock()) {
    EXPECT_EQ(set->size(), 1);
    EXPECT_TRUE(set->count(objectives_[0]));
  } else {
    FAIL() << "Unable to acquire lock";
  }
  EXPECT_EQ(sender_->dispose(e), arlcore::io::SendStatus::SUCCESS);
  sender_->waitForAcknowledgements(maxWait_);

  EXPECT_TRUE(setReader_->removeSetByMetadata(metadata));
  EXPECT_TRUE(result.set.expired());
  result = setReader_->getSetById(arlcore::NumericGuid(metadata.setID()));
  EXPECT_EQ(result.status, arlcore::umaa::LargeSetStatus::INVALID_SET);
}

TEST_F(LargeSetReaderTest, multiElementSet) {
  auto metadata = initializeMetadata();
  ObjectiveSetElement setElement[3];
  setElement[0] = initializeSetElement(metadata, objectives_[0]);
  setElement[1] = initializeSetElement(metadata, objectives_[1]);
  setElement[2] = initializeSetElement(metadata, objectives_[2]);
  sender_->send(setElement[0]);
  sender_->send(setElement[1]);
  sender_->send(setElement[2]);
  sender_->waitForAcknowledgements(maxWait_);

  auto result = setReader_->getSetFromMetadata(metadata);
  EXPECT_EQ(result.status, arlcore::umaa::LargeSetStatus::VALID_SET);
  if (auto set = result.set.lock()) {
    EXPECT_EQ(set->size(), 3);
    EXPECT_TRUE(set->count(objectives_[0]));
    EXPECT_TRUE(set->count(objectives_[1]));
    EXPECT_TRUE(set->count(objectives_[2]));
  } else {
    FAIL() << "Unable to acquire lock";
  }
}

TEST_F(LargeSetReaderTest, multiSet) {
  // Initialize data for 2 separate sets
  auto metadata_0 = initializeMetadata();
  auto metadata_1 = initializeMetadata();
  ObjectiveSetElement setElement[2][3];
  setElement[0][0] = initializeSetElement(metadata_0, objectives_[0]);
  setElement[0][1] = initializeSetElement(metadata_0, objectives_[1]);
  setElement[0][2] = initializeSetElement(metadata_0, objectives_[2]);
  setElement[1][0] = initializeSetElement(metadata_1, objectives_[2]);
  setElement[1][1] = initializeSetElement(metadata_1, objectives_[3]);
  setElement[1][2] = initializeSetElement(metadata_1, objectives_[4]);

  // Send elements of both sets, intermixed and out of order
  sender_->send(setElement[0][2]);
  sender_->send(setElement[1][1]);
  sender_->send(setElement[0][1]);
  sender_->send(setElement[1][0]);
  sender_->send(setElement[1][2]);
  sender_->send(setElement[0][0]);
  sender_->waitForAcknowledgements(maxWait_);

  auto result = setReader_->getSetFromMetadata(metadata_0);
  EXPECT_EQ(result.status, arlcore::umaa::LargeSetStatus::VALID_SET);
  if (auto set = result.set.lock()) {
    EXPECT_EQ(set->size(), 3);
    EXPECT_TRUE(set->count(objectives_[0]));
    EXPECT_TRUE(set->count(objectives_[1]));
    EXPECT_TRUE(set->count(objectives_[2]));
  } else {
    FAIL() << "Unable to acquire lock";
  }

  result = setReader_->getSetFromMetadata(metadata_1);
  EXPECT_EQ(result.status, arlcore::umaa::LargeSetStatus::VALID_SET);
  if (auto set = result.set.lock()) {
    EXPECT_EQ(set->size(), 3);
    EXPECT_TRUE(set->count(objectives_[2]));
    EXPECT_TRUE(set->count(objectives_[3]));
    EXPECT_TRUE(set->count(objectives_[4]));
  } else {
    FAIL() << "Unable to acquire lock";
  }
}

TEST_F(LargeSetReaderTest, disposeSetElement) {
  auto metadata = initializeMetadata();
  ObjectiveSetElement setElement[3];
  setElement[0] = initializeSetElement(metadata, objectives_[0]);
  setElement[1] = initializeSetElement(metadata, objectives_[1]);
  setElement[2] = initializeSetElement(metadata, objectives_[2]);
  sender_->send(setElement[0]);
  sender_->send(setElement[1]);
  sender_->send(setElement[2]);
  sender_->waitForAcknowledgements(maxWait_);

  auto result = setReader_->getSetFromMetadata(metadata);
  EXPECT_EQ(result.status, arlcore::umaa::LargeSetStatus::VALID_SET);
  if (auto set = result.set.lock()) {
    EXPECT_EQ(set->size(), 3);
    EXPECT_TRUE(set->count(objectives_[0]));
    EXPECT_TRUE(set->count(objectives_[1]));
    EXPECT_TRUE(set->count(objectives_[2]));
  } else {
    FAIL() << "Unable to acquire lock";
  }

  metadata.updateElementID(setElement[1].elementID());
  metadata.updateElementTimestamp().reset();
  metadata.size(metadata.size() - 1);
  sender_->dispose(setElement[1]);
  sender_->waitForAcknowledgements(maxWait_);

  result = setReader_->getSetFromMetadata(metadata);
  EXPECT_EQ(result.status, arlcore::umaa::LargeSetStatus::VALID_SET);
  if (auto set = result.set.lock()) {
    EXPECT_EQ(set->size(), 2);
    EXPECT_TRUE(set->count(objectives_[0]));
    EXPECT_TRUE(set->count(objectives_[2]));
  } else {
    FAIL() << "Unable to acquire lock";
  }
}

TEST_F(LargeSetReaderTest, getSetById) {
  auto metadata = initializeMetadata();
  ObjectiveSetElement setElement[3];
  setElement[0] = initializeSetElement(metadata, objectives_[0]);
  setElement[1] = initializeSetElement(metadata, objectives_[1]);
  setElement[2] = initializeSetElement(metadata, objectives_[2]);
  sender_->send(setElement[0]);
  sender_->send(setElement[1]);
  sender_->send(setElement[2]);
  sender_->waitForAcknowledgements(maxWait_);

  setReader_->getSetFromMetadata(metadata);

  arlcore::NumericGuid setId(metadata.setID());
  auto result = setReader_->getSetById(setId);
  EXPECT_EQ(result.status, arlcore::umaa::LargeSetStatus::VALID_SET);
  if (auto set = result.set.lock()) {
    EXPECT_EQ(set->size(), 3);
    EXPECT_TRUE(set->count(objectives_[0]));
    EXPECT_TRUE(set->count(objectives_[1]));
    EXPECT_TRUE(set->count(objectives_[2]));
  } else {
    FAIL() << "Unable to acquire lock";
  }
}

TEST_F(LargeSetReaderTest, getSetByBadId) {
  arlcore::NumericGuid setId = arlcore::NIL_GUID;
  auto result = setReader_->getSetById(setId);
  EXPECT_EQ(result.status, arlcore::umaa::LargeSetStatus::INVALID_SET);
  EXPECT_TRUE(result.set.expired());
}