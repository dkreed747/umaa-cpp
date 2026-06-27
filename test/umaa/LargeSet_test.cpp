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
#include <UMAA/MM/BaseType/TaskPlanType.hpp>

#include "LargeSet.h"
#include "UuidFactory.h"

using ObjectiveSetElement = UMAA::MM::BaseType::TaskPlanTypeObjectivesSetElement;
using UMAA::MM::BaseType::ObjectiveType;
using UMAA::Common::LargeSetMetadata;
using UMAA::Common::Measurement::DateTime;

class LargeSetTest : public ::testing::Test {
 protected:
  void SetUp() override {
    setId_ = arlcore::UuidFactory::getInstance().generateGuid();
    metadata_.setID(setId_.getGuid());
    metadata_.size(0);
    metadata_.updateElementID(arlcore::NIL_GUID.getGuid());
    objectives_[0] = initializeObjective("obj_1");
    objectives_[1] = initializeObjective("obj_2");
    objectives_[2] = initializeObjective("obj_3");
    objectives_[3] = initializeObjective("obj_4");
    objectives_[4] = initializeObjective("obj_5");
    set_ = std::make_unique<arlcore::umaa::LargeSet<ObjectiveType, ObjectiveSetElement>>(setId_);
  }

  void TearDown() override {
    // Code to run after running each TEST_F()
  }

  arlcore::NumericGuid setId_;
  LargeSetMetadata metadata_;
  ObjectiveType objectives_[5];
  std::unique_ptr<arlcore::umaa::LargeSet<ObjectiveType, ObjectiveSetElement>> set_;

  ObjectiveSetElement initializeSetElement(LargeSetMetadata& metadata, const ObjectiveType& element) {
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

  ObjectiveType initializeObjective(std::string name) {
    ObjectiveType obj;
    obj.name(name);
    obj.objectiveID(arlcore::UuidFactory::getInstance().generateGuid().getGuid());
    return obj;
  }
};

TEST_F(LargeSetTest, constructFromId) {
  arlcore::umaa::LargeSet<ObjectiveType, ObjectiveSetElement> set(setId_);
  EXPECT_EQ(setId_, set.getId());
  auto result = set.getSet();
  EXPECT_EQ(result.status, arlcore::umaa::LargeSetStatus::EMPTY_SET);
  EXPECT_TRUE(result.set.expired());
}

TEST_F(LargeSetTest, constructFromMetadata) {
  arlcore::umaa::LargeSet<ObjectiveType, ObjectiveSetElement> set(metadata_);
  EXPECT_EQ(setId_, set.getId());
  auto result = set.getSet();
  EXPECT_EQ(result.status, arlcore::umaa::LargeSetStatus::EMPTY_SET);
  EXPECT_TRUE(result.set.expired());
}

TEST_F(LargeSetTest, singleElementSet) {
  ObjectiveSetElement e = initializeSetElement(metadata_, objectives_[0]);
  EXPECT_TRUE(set_->receive(e));
  auto result = set_->receive(metadata_);
  EXPECT_EQ(result.status, arlcore::umaa::LargeSetStatus::VALID_SET);
  EXPECT_FALSE(result.set.expired());
  if (auto set = result.set.lock()) {
    EXPECT_EQ(set->size(), 1);
    EXPECT_TRUE(set->count(objectives_[0]));
  } else {
    FAIL() << "Unable to acquire lock";
  }
}

TEST_F(LargeSetTest, invalidSetElement) {
  ObjectiveSetElement e = initializeSetElement(metadata_, objectives_[0]);
  e.setID(arlcore::NIL_GUID.getGuid());
  EXPECT_FALSE(set_->receive(e));
}

TEST_F(LargeSetTest, invalidMetadata) {
  metadata_.setID(arlcore::NIL_GUID.getGuid());
  auto result = set_->receive(metadata_);
  EXPECT_EQ(result.status, arlcore::umaa::LargeSetStatus::INVALID_SET);
  EXPECT_TRUE(result.set.expired());
}

TEST_F(LargeSetTest, multiElementSet) {
  ObjectiveSetElement setElement[3];
  setElement[0] = initializeSetElement(metadata_, objectives_[0]);
  setElement[1] = initializeSetElement(metadata_, objectives_[1]);
  setElement[2] = initializeSetElement(metadata_, objectives_[2]);
  EXPECT_TRUE(set_->receive(setElement[0]));
  EXPECT_TRUE(set_->receive(setElement[1]));
  EXPECT_TRUE(set_->receive(setElement[2]));
  auto result = set_->receive(metadata_);
  EXPECT_EQ(result.status, arlcore::umaa::LargeSetStatus::VALID_SET);
  EXPECT_FALSE(result.set.expired());
  if (auto set = result.set.lock()) {
    EXPECT_EQ(set->size(), 3);
    EXPECT_TRUE(set->count(objectives_[0]));
    EXPECT_TRUE(set->count(objectives_[1]));
    EXPECT_TRUE(set->count(objectives_[2]));
  } else {
    FAIL() << "Unable to accquire lock";
  }
}

TEST_F(LargeSetTest, invalidSetMissingElement) {
  ObjectiveSetElement setElement[3];
  setElement[0] = initializeSetElement(metadata_, objectives_[0]);
  setElement[1] = initializeSetElement(metadata_, objectives_[1]);
  setElement[2] = initializeSetElement(metadata_, objectives_[2]);
  EXPECT_TRUE(set_->receive(setElement[0]));
  EXPECT_TRUE(set_->receive(setElement[1]));

  // Set should be invalid since we're missing an element
  auto result = set_->receive(metadata_);
  EXPECT_EQ(result.status, arlcore::umaa::LargeSetStatus::INVALID_SET);
  EXPECT_TRUE(result.set.expired());

  // Update the set to fix the broken link
  EXPECT_TRUE(set_->receive(setElement[2]));
  result = set_->receive(metadata_);
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

TEST_F(LargeSetTest, invalidSetUpdateInProgress) {
  ObjectiveSetElement setElement[3];
  setElement[0] = initializeSetElement(metadata_, objectives_[0]);
  setElement[1] = initializeSetElement(metadata_, objectives_[1]);

  // Update the timestamp to be later than the ending timestamp to simulate receiving an update after getting the
  // metadata
  auto originalTimestamp = metadata_.updateElementTimestamp().value();
  setElement[1].elementTimestamp(DateTime(100, 0));

  setElement[2] = initializeSetElement(metadata_, objectives_[2]);
  EXPECT_TRUE(set_->receive(setElement[0]));
  EXPECT_TRUE(set_->receive(setElement[1]));
  EXPECT_TRUE(set_->receive(setElement[2]));

  // Set should be invalid since there's an element has been updated since the metadata was received
  auto result = set_->receive(metadata_);
  EXPECT_EQ(result.status, arlcore::umaa::LargeSetStatus::INVALID_SET);
  EXPECT_TRUE(result.set.expired());

  // Change the timestamp to pretend there was no update
  setElement[1].elementTimestamp(originalTimestamp);
  EXPECT_TRUE(set_->receive(setElement[1]));
  result = set_->receive(metadata_);
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

TEST_F(LargeSetTest, updateSetElement) {
  ObjectiveSetElement setElement[3];
  setElement[0] = initializeSetElement(metadata_, objectives_[0]);
  setElement[1] = initializeSetElement(metadata_, objectives_[1]);
  setElement[2] = initializeSetElement(metadata_, objectives_[2]);
  EXPECT_TRUE(set_->receive(setElement[0]));
  EXPECT_TRUE(set_->receive(setElement[1]));
  EXPECT_TRUE(set_->receive(setElement[2]));

  auto result = set_->receive(metadata_);
  EXPECT_EQ(result.status, arlcore::umaa::LargeSetStatus::VALID_SET);
  if (auto set = result.set.lock()) {
    EXPECT_EQ(set->size(), 3);
    EXPECT_TRUE(set->count(objectives_[0]));
    EXPECT_TRUE(set->count(objectives_[1]));
    EXPECT_TRUE(set->count(objectives_[2]));
  } else {
    FAIL() << "Unable to acquire lock";
  }

  metadata_.updateElementTimestamp(DateTime(metadata_.updateElementTimestamp().value().seconds() + 1, 0));
  metadata_.updateElementID(setElement[1].elementID());
  setElement[1].element(objectives_[3]);
  setElement[1].elementTimestamp(metadata_.updateElementTimestamp().value());
  EXPECT_TRUE(set_->receive(setElement[1]));
  result = set_->receive(metadata_);
  EXPECT_EQ(result.status, arlcore::umaa::LargeSetStatus::VALID_SET);
  if (auto set = result.set.lock()) {
    EXPECT_EQ(set->size(), 3);
    EXPECT_TRUE(set->count(objectives_[0]));
    EXPECT_TRUE(set->count(objectives_[3]));
    EXPECT_TRUE(set->count(objectives_[2]));
  } else {
    FAIL() << "Unable to acquire lock";
  }
}

TEST_F(LargeSetTest, disposeSetElement) {
  ObjectiveSetElement setElement[3];
  setElement[0] = initializeSetElement(metadata_, objectives_[0]);
  setElement[1] = initializeSetElement(metadata_, objectives_[1]);
  setElement[2] = initializeSetElement(metadata_, objectives_[2]);
  EXPECT_TRUE(set_->receive(setElement[0]));
  EXPECT_TRUE(set_->receive(setElement[1]));
  EXPECT_TRUE(set_->receive(setElement[2]));
  auto result = set_->receive(metadata_);
  EXPECT_EQ(result.status, arlcore::umaa::LargeSetStatus::VALID_SET);
  if (auto set = result.set.lock()) {
    EXPECT_EQ(set->size(), 3);
    EXPECT_TRUE(set->count(objectives_[0]));
    EXPECT_TRUE(set->count(objectives_[1]));
    EXPECT_TRUE(set->count(objectives_[2]));
  } else {
    FAIL() << "Unable to acquire lock";
  }

  setElement[0].elementTimestamp(DateTime(metadata_.updateElementTimestamp().value().seconds() + 1, 0));
  metadata_.updateElementID(setElement[1].elementID());
  metadata_.updateElementTimestamp().reset();
  metadata_.size(metadata_.size() - 1);
  EXPECT_TRUE(set_->receive(setElement[0]));
  EXPECT_TRUE(set_->dispose(setElement[1]));
  result = set_->receive(metadata_);
  EXPECT_EQ(result.status, arlcore::umaa::LargeSetStatus::VALID_SET);
  if (auto set = result.set.lock()) {
    EXPECT_EQ(set->size(), 2);
    EXPECT_TRUE(set->count(objectives_[0]));
    EXPECT_TRUE(set->count(objectives_[2]));
  } else {
    FAIL() << "Unable to acquire lock";
  }
}

TEST_F(LargeSetTest, setCache) {
  ObjectiveSetElement setElement[3];
  setElement[0] = initializeSetElement(metadata_, objectives_[0]);
  setElement[1] = initializeSetElement(metadata_, objectives_[1]);
  setElement[2] = initializeSetElement(metadata_, objectives_[2]);
  set_->receive(setElement[0]);
  set_->receive(setElement[1]);
  set_->receive(setElement[2]);
  set_->receive(metadata_);
  
  // Make changes to the set after latest "commit"
  setElement[0].elementTimestamp(DateTime(metadata_.updateElementTimestamp().value().seconds() + 1, 0));
  metadata_.updateElementID(setElement[1].elementID());
  metadata_.updateElementTimestamp().reset();
  metadata_.size(metadata_.size() - 1);
  set_->receive(setElement[0]);
  set_->dispose(setElement[1]);
  
  // Should still be original set
  auto result = set_->getSet();
  EXPECT_EQ(result.status, arlcore::umaa::LargeSetStatus::VALID_SET);
  if (auto set = result.set.lock()) {
    EXPECT_EQ(set->size(), 3);
    EXPECT_TRUE(set->count(objectives_[0]));
    EXPECT_TRUE(set->count(objectives_[1]));
    EXPECT_TRUE(set->count(objectives_[2]));
  } else {
    FAIL() << "Unable to acquire lock";
  }

  // Now when we update the metadata, we get the new set
  result = set_->receive(metadata_);
  EXPECT_EQ(result.status, arlcore::umaa::LargeSetStatus::VALID_SET);
  if (auto set = result.set.lock()) {
    EXPECT_EQ(set->size(), 2);
    EXPECT_TRUE(set->count(objectives_[0]));
    EXPECT_TRUE(set->count(objectives_[2]));
  } else {
    FAIL() << "Unable to acquire lock";
  }
}