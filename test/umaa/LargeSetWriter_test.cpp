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

#include "LargeSetWriter.h"

#include <gtest/gtest.h>

#include <UMAA/MM/BaseType/TaskPlanType.hpp>
#include <dds/dds.hpp>

#include "CycloneReader.h"
#include "CycloneSender.h"
#include "UuidFactory.h"

using ObjectiveSetElement = UMAA::MM::BaseType::TaskPlanTypeObjectivesSetElement;
using UMAA::MM::BaseType::TaskPlanTypeObjectivesSetElementTopic;
using UMAA::MM::BaseType::ObjectiveType;
using UMAA::Common::LargeSetMetadata;
using UMAA::Common::Measurement::DateTime;


class LargeSetWriterTest : public ::testing::Test {

 protected:
  const int32_t DOMAIN_ID = 79;
  const std::string SET_TOPIC = TaskPlanTypeObjectivesSetElementTopic;

  void SetUpDDS() {
    participant_ = arlcore::io::getDomainParticipant(DOMAIN_ID);
    // QOS
    dds::sub::qos::DataReaderQos rQos;
    rQos << dds::core::policy::Reliability::Reliable(dds::core::Duration::from_secs(10))
         << dds::core::policy::History::KeepAll()
         << dds::core::policy::Durability::TransientLocal()
         << dds::core::policy::DestinationOrder::SourceTimestamp();
    dds::pub::qos::DataWriterQos wQos;
    wQos << dds::core::policy::Reliability::Reliable(dds::core::Duration::from_secs(10))
         << dds::core::policy::History::KeepAll()
         << dds::core::policy::Durability::TransientLocal()
         << dds::core::policy::DestinationOrder::SourceTimestamp();

    // Create Topic
    auto setTopic = arlcore::io::getTopic<ObjectiveSetElement>(participant_, SET_TOPIC);
    auto subscriber = arlcore::io::createSubscriber(participant_);
    auto publisher = arlcore::io::createPublisher(participant_);
    reader_ = dds::sub::DataReader<ObjectiveSetElement>(subscriber,
                                                        setTopic,
                                                        rQos);
    sender_ =  std::make_shared<arlcore::io::CycloneSender<ObjectiveSetElement>>(
        participant_, SET_TOPIC, wQos);
  }
  
  void SetUp() override {
    SetUpDDS();
    objectives_[0] = initializeObjective("obj_1");
    objectives_[1] = initializeObjective("obj_2");
    objectives_[2] = initializeObjective("obj_3");
    objectives_[3] = initializeObjective("obj_4");
    objectives_[4] = initializeObjective("obj_5");
    usleep(30000);  // Discovery action

    setWriter_ = std::make_unique<arlcore::umaa::LargeSetWriter<ObjectiveType, ObjectiveSetElement>>(sender_);
    auto samples = reader_.take();
  }

  dds::domain::DomainParticipant participant_ = dds::core::null;
  dds::sub::DataReader<ObjectiveSetElement> reader_ = dds::core::null;
  std::shared_ptr<arlcore::io::SenderBase<ObjectiveSetElement>> sender_;
  std::unique_ptr<arlcore::umaa::LargeSetWriter<ObjectiveType, ObjectiveSetElement>> setWriter_;
  ObjectiveType objectives_[5];

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
};

TEST_F(LargeSetWriterTest, emptySet) {
  auto metadata = setWriter_->getMetadata();
  EXPECT_EQ(metadata.updateElementID(), arlcore::NIL_GUID.getGuid());
  EXPECT_EQ(metadata.size(), 0);
  EXPECT_EQ(setWriter_->size(), 0);
  EXPECT_FALSE(metadata.updateElementTimestamp().has_value());
}

TEST_F(LargeSetWriterTest, initialSet) {
  std::vector<ObjectiveType> objectives;
  objectives.push_back(objectives_[0]);
  objectives.push_back(objectives_[1]);
  arlcore::umaa::LargeSetWriter<ObjectiveType, ObjectiveSetElement> setWriter(sender_, objectives);
  auto metadata = setWriter.getMetadata();
  EXPECT_EQ(metadata.size(), 2);
  EXPECT_EQ(setWriter.size(), 2);

  auto samples = reader_.take();
  auto sample = samples.begin();
  ASSERT_EQ(samples.length(), 2);
  EXPECT_EQ(sample[0].data().element(), objectives_[0]);
  EXPECT_EQ(sample[1].data().element(), objectives_[1]);
  EXPECT_EQ(sample[0].data().setID(), setWriter.getSetId().getGuid());
  EXPECT_EQ(sample[1].data().setID(), setWriter.getSetId().getGuid());
  EXPECT_EQ(sample[1].data().elementTimestamp(), metadata.updateElementTimestamp().value());
  EXPECT_EQ(sample[1].data().elementID(), metadata.updateElementID());
  
}

TEST_F(LargeSetWriterTest, clear) {
  EXPECT_EQ(setWriter_->insert(objectives_[0]), arlcore::io::SendStatus::SUCCESS);
  EXPECT_EQ(setWriter_->insert(objectives_[1]), arlcore::io::SendStatus::SUCCESS);

  auto samples = reader_.take();
  auto sample = samples.begin();
  ASSERT_EQ(samples.length(), 2);
  std::set<UMAA::Common::Measurement::NumericGUID> ids;
  ids.insert(sample[0].data().elementID());
  ids.insert(sample[1].data().elementID());
  

  EXPECT_EQ(setWriter_->clear(), arlcore::io::SendStatus::SUCCESS);
  auto metadata = setWriter_->getMetadata();
  EXPECT_EQ(metadata.size(), 0);
  EXPECT_FALSE(metadata.updateElementTimestamp().has_value());

  samples = reader_.take();
  sample = samples.begin();
  ASSERT_EQ(samples.length(), 2);
  ObjectiveSetElement keySample0, keySample1;
  reader_.key_value(keySample0, sample[0].info().instance_handle());
  reader_.key_value(keySample1, sample[1].info().instance_handle());
  EXPECT_TRUE(ids.count(keySample0.elementID()));
  EXPECT_TRUE(ids.count(keySample1.elementID()));
  
  EXPECT_EQ(setWriter_->size(), 0);
}

TEST_F(LargeSetWriterTest, insert) {
  EXPECT_EQ(setWriter_->insert(objectives_[0]), arlcore::io::SendStatus::SUCCESS);
  EXPECT_EQ(setWriter_->insert(objectives_[1]), arlcore::io::SendStatus::SUCCESS);
  auto metadata = setWriter_->getMetadata();

  auto samples = reader_.take();
  auto sample = samples.begin();
  ASSERT_EQ(samples.length(), 2);
  EXPECT_EQ(sample[0].data().element(), objectives_[0]);
  EXPECT_EQ(sample[1].data().element(), objectives_[1]);
  EXPECT_EQ(sample[1].data().elementID(), metadata.updateElementID());
  EXPECT_EQ(sample[1].data().elementTimestamp(), metadata.updateElementTimestamp().value());
  
}

TEST_F(LargeSetWriterTest, insertVector) {
  std::vector<ObjectiveType> objectives(std::begin(objectives_), std::end(objectives_));
  EXPECT_EQ(setWriter_->insert(objectives), arlcore::io::SendStatus::SUCCESS);
  auto metadata = setWriter_->getMetadata();

  auto samples = reader_.take();
  auto sample = samples.begin();
  ASSERT_EQ(samples.length(), 5);
  EXPECT_EQ(sample[0].data().element(), objectives_[0]);
  EXPECT_EQ(sample[1].data().element(), objectives_[1]);
  EXPECT_EQ(sample[2].data().element(), objectives_[2]);
  EXPECT_EQ(sample[3].data().element(), objectives_[3]);
  EXPECT_EQ(sample[4].data().element(), objectives_[4]);
  EXPECT_EQ(sample[4].data().elementID(), metadata.updateElementID());
  EXPECT_EQ(sample[4].data().elementTimestamp(), metadata.updateElementTimestamp().value());
  
}

TEST_F(LargeSetWriterTest, update) {
  EXPECT_EQ(setWriter_->insert(objectives_[0]), arlcore::io::SendStatus::SUCCESS);
  EXPECT_EQ(setWriter_->update(objectives_[0], objectives_[1]), arlcore::io::SendStatus::SUCCESS);
  auto metadata = setWriter_->getMetadata();
  auto samples = reader_.take();
  auto sample = samples.begin();
  ASSERT_EQ(samples.length(), 2);
  EXPECT_EQ(sample[0].data().element(), objectives_[0]);
  EXPECT_EQ(sample[1].data().element(), objectives_[1]);
  EXPECT_EQ(sample[0].data().elementID(), sample[1].data().elementID());
  EXPECT_EQ(sample[1].data().elementID(), metadata.updateElementID());
  EXPECT_EQ(sample[1].data().elementTimestamp(), metadata.updateElementTimestamp());
  
}

TEST_F(LargeSetWriterTest, remove) {
  EXPECT_EQ(setWriter_->insert(objectives_[0]), arlcore::io::SendStatus::SUCCESS);
  EXPECT_EQ(setWriter_->insert(objectives_[1]), arlcore::io::SendStatus::SUCCESS);
  EXPECT_EQ(setWriter_->insert(objectives_[2]), arlcore::io::SendStatus::SUCCESS);

  auto samples = reader_.take();
  auto sample = samples.begin();
  ASSERT_EQ(samples.length(), 3);
  EXPECT_EQ(sample[0].data().element(), objectives_[0]);
  EXPECT_EQ(sample[1].data().element(), objectives_[1]);
  EXPECT_EQ(sample[2].data().element(), objectives_[2]);
  auto updateId = sample[1].data().elementID();
  

  EXPECT_EQ(setWriter_->remove(objectives_[1]), arlcore::io::SendStatus::SUCCESS);
  auto metadata = setWriter_->getMetadata();
  EXPECT_FALSE(metadata.updateElementTimestamp().has_value());

  samples = reader_.take();
  sample = samples.begin();
  ASSERT_EQ(samples.length(), 1);
  EXPECT_EQ(sample[0].info().state().instance_state(), dds::sub::status::InstanceState::not_alive_disposed());
  ObjectiveSetElement keySample;
  reader_.key_value(keySample, sample[0].info().instance_handle());
  EXPECT_EQ(keySample.elementID(), metadata.updateElementID());
}

TEST_F(LargeSetWriterTest, findSetElementIf) {
  EXPECT_EQ(setWriter_->insert(objectives_[0]), arlcore::io::SendStatus::SUCCESS);
  EXPECT_EQ(setWriter_->insert(objectives_[1]), arlcore::io::SendStatus::SUCCESS);
  EXPECT_EQ(setWriter_->insert(objectives_[2]), arlcore::io::SendStatus::SUCCESS);

  std::string name_match = objectives_[1].name();
  auto setElem = setWriter_->findSetElementIf([&name_match](const ObjectiveType& obj) {
    return obj.name() == name_match;});
  ASSERT_TRUE(setElem.has_value());
  EXPECT_EQ(setElem.value().element(), objectives_[1]);

  // Perform simple operations using the info we got back
  EXPECT_EQ(setWriter_->getElementById(arlcore::NumericGuid(setElem.value().elementID())).value(),
            objectives_[1]);
}