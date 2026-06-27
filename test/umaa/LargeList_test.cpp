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
#include <UMAA/MO/GlobalWaypointControl/GlobalWaypointCommandType.hpp>

#include "LargeList.h"
#include "UuidFactory.h"

using WaypointListElement = UMAA::MO::GlobalWaypointControl::GlobalWaypointCommandTypeWaypointsListElement;
using UMAA::MO::GlobalWaypointControl::GlobalWaypointType;
using UMAA::Common::LargeListMetadata;
using UMAA::Common::Measurement::DateTime;

class LargeListTest : public ::testing::Test {
 protected:
  void SetUp() override {
    listId_ = arlcore::UuidFactory::getInstance().generateGuid();
    metadata_.listID(listId_.getGuid());
    metadata_.size(0);
    metadata_.updateElementID(arlcore::NIL_GUID.getGuid());
    metadata_.startingElementID(arlcore::NIL_GUID.getGuid());
    waypoints_[0] = initializeWaypoint(1.2, 12.345, -12.345, 2.345);
    waypoints_[1] = initializeWaypoint(2.2, 13.345, -13.345, 3.345);
    waypoints_[2] = initializeWaypoint(3.2, 14.345, -14.345, 4.345);
    waypoints_[3] = initializeWaypoint(4.2, 15.345, -15.345, 5.345);
    waypoints_[4] = initializeWaypoint(5.2, 16.345, -16.345, 6.345);
    list_ = std::make_unique<arlcore::umaa::LargeList<GlobalWaypointType, WaypointListElement>>(listId_);
  }

  void TearDown() override {
    // Code to run after running each TEST_F()
  }

  arlcore::NumericGuid listId_;
  LargeListMetadata metadata_;
  GlobalWaypointType waypoints_[5];
  std::unique_ptr<arlcore::umaa::LargeList<GlobalWaypointType, WaypointListElement>> list_;

  WaypointListElement initializeListElement(LargeListMetadata& metadata, const GlobalWaypointType& element) {
    static uint64_t timestamp_sec = 0;
    WaypointListElement e;
    e.listID(metadata.listID());
    e.elementID(arlcore::UuidFactory::getInstance().generateGuid().getGuid());
    e.element(element);
    e.elementTimestamp(DateTime(timestamp_sec, 0));
    metadata.size(metadata.size() + 1);
    metadata.updateElementID(e.elementID());
    metadata.updateElementTimestamp(e.elementTimestamp());
    if (arlcore::NIL_GUID == metadata.startingElementID()) {
      metadata.startingElementID(e.elementID());
    }
    timestamp_sec++;
    return e;
}

  GlobalWaypointType initializeWaypoint(flt64_t depth, flt64_t lat, flt64_t lon, flt64_t speed) {
    GlobalWaypointType wp;

    // Elevation is an optional so must be set like this...

    UMAA::Common::Measurement::ElevationRequirementVariantType depthVariant;
    depthVariant.ElevationRequirementVariantTypeSubtypes().DepthRequirementVariantVariant(UMAA::Common::Measurement::DepthRequirementVariantType());
    depthVariant.ElevationRequirementVariantTypeSubtypes().DepthRequirementVariantVariant().depth().depth(depth);
    wp.elevation(depthVariant);
    
    UMAA::Common::Measurement::GeoPosition2D latLon(lat, lon);
    UMAA::Common::Position::GeoPosition2DRequirement latLonReq;
    latLonReq.value(latLon);
    wp.position(latLonReq);


    // Speed control is also an optional
    UMAA::Common::Speed::VariableSpeedVariantType speedControl;

    speedControl.VariableSpeedVariantTypeSubtypes().RequiredSpeedVariantVariant(UMAA::Common::Speed::RequiredSpeedVariantType());

    speedControl.VariableSpeedVariantTypeSubtypes()
        .RequiredSpeedVariantVariant()
        .speed()
        .SpeedRequirementVariantTypeSubtypes().GroundSpeedRequirementVariantVariant(UMAA::Common::Speed::GroundSpeedRequirementVariantType());

    speedControl.VariableSpeedVariantTypeSubtypes()
        .RequiredSpeedVariantVariant()
        .speed()
        .SpeedRequirementVariantTypeSubtypes()
        .GroundSpeedRequirementVariantVariant()
        .speed()
        .speed(speed);
    wp.speed(speedControl);

    wp.waypointID(arlcore::UuidFactory::getInstance().generateGuid().getGuid());

    return wp;
  }
};

TEST_F(LargeListTest, constructFromId) {
  arlcore::umaa::LargeList<GlobalWaypointType, WaypointListElement> list(listId_);
  EXPECT_EQ(listId_, list.getId());
  auto result = list.getList();
  EXPECT_EQ(result.status, arlcore::umaa::LargeListStatus::EMPTY_LIST);
  EXPECT_TRUE(result.list.expired());
}

TEST_F(LargeListTest, constructFromMetadata) {
  arlcore::umaa::LargeList<GlobalWaypointType, WaypointListElement> list(metadata_);
  EXPECT_EQ(listId_, list.getId());
  auto result = list.getList();
  EXPECT_EQ(result.status, arlcore::umaa::LargeListStatus::EMPTY_LIST);
  EXPECT_TRUE(result.list.expired());
}

TEST_F(LargeListTest, singleElementList) {
  WaypointListElement e = initializeListElement(metadata_, waypoints_[0]);
  EXPECT_TRUE(list_->receive(e));
  auto result = list_->receive(metadata_);
  EXPECT_EQ(result.status, arlcore::umaa::LargeListStatus::VALID_LIST);
  EXPECT_FALSE(result.list.expired());
  if (auto list = result.list.lock()) {
    EXPECT_EQ(list->size(), 1);
    EXPECT_EQ(list->front(), waypoints_[0]);
  } else {
    FAIL() << "Unable to acquire lock";
  }
}

TEST_F(LargeListTest, invalidListElement) {
  WaypointListElement e = initializeListElement(metadata_, waypoints_[0]);
  e.listID(arlcore::NIL_GUID.getGuid());
  EXPECT_FALSE(list_->receive(e));
}

TEST_F(LargeListTest, invalidMetadata) {
  metadata_.listID(arlcore::NIL_GUID.getGuid());
  auto result = list_->receive(metadata_);
  EXPECT_EQ(result.status, arlcore::umaa::LargeListStatus::INVALID_LIST);
  EXPECT_TRUE(result.list.expired());
}

TEST_F(LargeListTest, multiElementList) {
  WaypointListElement listElement[3];
  listElement[0] = initializeListElement(metadata_, waypoints_[0]);
  listElement[1] = initializeListElement(metadata_, waypoints_[1]);
  listElement[2] = initializeListElement(metadata_, waypoints_[2]);
  listElement[0].nextElementID(listElement[1].elementID());
  listElement[1].nextElementID(listElement[2].elementID());
  EXPECT_TRUE(list_->receive(listElement[0]));
  EXPECT_TRUE(list_->receive(listElement[1]));
  EXPECT_TRUE(list_->receive(listElement[2]));
  auto result = list_->receive(metadata_);
  EXPECT_EQ(result.status, arlcore::umaa::LargeListStatus::VALID_LIST);
  EXPECT_FALSE(result.list.expired());
  if (auto list = result.list.lock()) {
    EXPECT_EQ(list->size(), 3);
    auto it = list->begin();
    EXPECT_EQ(*it, waypoints_[0]);
    it = std::next(it);
    EXPECT_EQ(*it, waypoints_[1]);
    it = std::next(it);
    EXPECT_EQ(*it, waypoints_[2]);
  } else {
    FAIL() << "Unable to acquire lock";
  }
}

TEST_F(LargeListTest, invalidListLoop) {
  WaypointListElement listElement[3];
  listElement[0] = initializeListElement(metadata_, waypoints_[0]);
  listElement[1] = initializeListElement(metadata_, waypoints_[1]);
  listElement[2] = initializeListElement(metadata_, waypoints_[2]);
  listElement[0].nextElementID(listElement[1].elementID());
  listElement[1].nextElementID(listElement[2].elementID());
  listElement[2].nextElementID(listElement[0].elementID());
  EXPECT_TRUE(list_->receive(listElement[0]));
  EXPECT_TRUE(list_->receive(listElement[1]));
  EXPECT_TRUE(list_->receive(listElement[2]));
  auto result = list_->receive(metadata_);
  EXPECT_EQ(result.status, arlcore::umaa::LargeListStatus::INVALID_LIST);
  EXPECT_TRUE(result.list.expired());

  // Update the list to fix the broken link
  listElement[2].nextElementID(arlcore::NIL_GUID.getGuid());
  auto latest = metadata_.updateElementTimestamp().value();
  latest.seconds(latest.seconds() + 1);
  listElement[2].elementTimestamp(latest);
  metadata_.updateElementTimestamp(latest);
  EXPECT_TRUE(list_->receive(listElement[2]));
  result = list_->receive(metadata_);
  EXPECT_EQ(result.status, arlcore::umaa::LargeListStatus::VALID_LIST);
  EXPECT_FALSE(result.list.expired());
  if (auto list = result.list.lock()) {
    EXPECT_EQ(list->size(), 3);
    auto it = list->begin();
    EXPECT_EQ(*it, waypoints_[0]);
    it = std::next(it);
    EXPECT_EQ(*it, waypoints_[1]);
    it = std::next(it);
    EXPECT_EQ(*it, waypoints_[2]);
  } else {
    FAIL() << "Unable to acquire lock";
  }
}

TEST_F(LargeListTest, invalidListBrokenLink) {
  WaypointListElement listElement[3];
  listElement[0] = initializeListElement(metadata_, waypoints_[0]);
  listElement[1] = initializeListElement(metadata_, waypoints_[1]);
  listElement[2] = initializeListElement(metadata_, waypoints_[2]);
  listElement[0].nextElementID(listElement[1].elementID());
  EXPECT_TRUE(list_->receive(listElement[0]));
  EXPECT_TRUE(list_->receive(listElement[1]));
  EXPECT_TRUE(list_->receive(listElement[2]));
  auto result = list_->receive(metadata_);
  EXPECT_EQ(result.status, arlcore::umaa::LargeListStatus::INVALID_LIST);
  EXPECT_TRUE(result.list.expired());

  // Update the list to fix the broken link
  listElement[1].nextElementID(listElement[2].elementID());
  auto latest = metadata_.updateElementTimestamp().value();
  latest.seconds(latest.seconds() + 1);
  listElement[1].elementTimestamp(latest);
  metadata_.updateElementTimestamp(latest);
  EXPECT_TRUE(list_->receive(listElement[1]));
  result = list_->receive(metadata_);
  EXPECT_EQ(result.status, arlcore::umaa::LargeListStatus::VALID_LIST);
  EXPECT_FALSE(result.list.expired());
  if (auto list = result.list.lock()) {
    EXPECT_EQ(list->size(), 3);
    auto it = list->begin();
    EXPECT_EQ(*it, waypoints_[0]);
    it = std::next(it);
    EXPECT_EQ(*it, waypoints_[1]);
    it = std::next(it);
    EXPECT_EQ(*it, waypoints_[2]);
  } else {
    FAIL() << "Unable to acquire lock";
  }
}

TEST_F(LargeListTest, invalidListMissingElement) {
  WaypointListElement listElement[3];
  listElement[0] = initializeListElement(metadata_, waypoints_[0]);
  listElement[1] = initializeListElement(metadata_, waypoints_[1]);
  listElement[2] = initializeListElement(metadata_, waypoints_[2]);
  listElement[0].nextElementID(listElement[1].elementID());
  listElement[1].nextElementID(listElement[2].elementID());
  EXPECT_TRUE(list_->receive(listElement[0]));
  EXPECT_TRUE(list_->receive(listElement[1]));

  // List should be invalid since we're missing an element
  auto result = list_->receive(metadata_);
  EXPECT_EQ(result.status, arlcore::umaa::LargeListStatus::INVALID_LIST);
  EXPECT_TRUE(result.list.expired());

  // Update the list to fix the broken link
  EXPECT_TRUE(list_->receive(listElement[2]));
  result = list_->receive(metadata_);
  EXPECT_EQ(result.status, arlcore::umaa::LargeListStatus::VALID_LIST);
  if (auto list = result.list.lock()) {
    EXPECT_EQ(list->size(), 3);
    auto it = list->begin();
    EXPECT_EQ(*it, waypoints_[0]);
    it = std::next(it);
    EXPECT_EQ(*it, waypoints_[1]);
    it = std::next(it);
    EXPECT_EQ(*it, waypoints_[2]);
  } else {
    FAIL() << "Unable to acquire lock";
  }
}

TEST_F(LargeListTest, invalidListUpdateInProgress) {
  WaypointListElement listElement[3];
  listElement[0] = initializeListElement(metadata_, waypoints_[0]);
  listElement[1] = initializeListElement(metadata_, waypoints_[1]);

  // Update the timestamp to be later than the ending timestamp to simulate receiving an update after getting the
  // metadata
  auto originalTimestamp = metadata_.updateElementTimestamp().value();
  listElement[1].elementTimestamp(DateTime(100, 0));

  listElement[2] = initializeListElement(metadata_, waypoints_[2]);
  listElement[0].nextElementID(listElement[1].elementID());
  listElement[1].nextElementID(listElement[2].elementID());
  EXPECT_TRUE(list_->receive(listElement[0]));
  EXPECT_TRUE(list_->receive(listElement[1]));
  EXPECT_TRUE(list_->receive(listElement[2]));

  // List should be invalid since there's an element has been updated since the metadata was received
  auto result = list_->receive(metadata_);
  EXPECT_EQ(result.status, arlcore::umaa::LargeListStatus::INVALID_LIST);
  EXPECT_TRUE(result.list.expired());

  // Change the timestamp to pretend there was no update
  listElement[1].elementTimestamp(originalTimestamp);
  EXPECT_TRUE(list_->receive(listElement[1]));
  result = list_->receive(metadata_);
  EXPECT_EQ(result.status, arlcore::umaa::LargeListStatus::VALID_LIST);
  if (auto list = result.list.lock()) {
    EXPECT_EQ(list->size(), 3);
    auto it = list->begin();
    EXPECT_EQ(*it, waypoints_[0]);
    it = std::next(it);
    EXPECT_EQ(*it, waypoints_[1]);
    it = std::next(it);
    EXPECT_EQ(*it, waypoints_[2]);
  } else {
    FAIL() << "Unable to acquire lock";
  }
}

TEST_F(LargeListTest, updateListElement) {
  WaypointListElement listElement[3];
  listElement[0] = initializeListElement(metadata_, waypoints_[0]);
  listElement[1] = initializeListElement(metadata_, waypoints_[1]);
  listElement[2] = initializeListElement(metadata_, waypoints_[2]);
  listElement[0].nextElementID(listElement[1].elementID());
  listElement[1].nextElementID(listElement[2].elementID());
  EXPECT_TRUE(list_->receive(listElement[0]));
  EXPECT_TRUE(list_->receive(listElement[1]));
  EXPECT_TRUE(list_->receive(listElement[2]));

  auto result = list_->receive(metadata_);
  EXPECT_EQ(result.status, arlcore::umaa::LargeListStatus::VALID_LIST);
  if (auto list = result.list.lock()) {
    EXPECT_EQ(list->size(), 3);
    auto it = list->begin();
    EXPECT_EQ(*it, waypoints_[0]);
    it = std::next(it);
    EXPECT_EQ(*it, waypoints_[1]);
    it = std::next(it);
    EXPECT_EQ(*it, waypoints_[2]);
  } else {
    FAIL() << "Unable to acquire lock";
  }

  metadata_.updateElementTimestamp(DateTime(metadata_.updateElementTimestamp().value().seconds() + 1, 0));
  metadata_.updateElementID(listElement[1].elementID());
  listElement[1].element(waypoints_[3]);
  listElement[1].elementTimestamp(metadata_.updateElementTimestamp().value());
  EXPECT_TRUE(list_->receive(listElement[1]));
  result = list_->receive(metadata_);
  EXPECT_EQ(result.status, arlcore::umaa::LargeListStatus::VALID_LIST);
  if (auto list = result.list.lock()) {
    EXPECT_EQ(list->size(), 3);
    auto it = list->begin();
    EXPECT_EQ(*it, waypoints_[0]);
    it = std::next(it);
    EXPECT_EQ(*it, waypoints_[3]);
    it = std::next(it);
    EXPECT_EQ(*it, waypoints_[2]);
  } else {
    FAIL() << "Unable to acquire lock";
  }
}

TEST_F(LargeListTest, disposeListElement) {
  WaypointListElement listElement[3];
  listElement[0] = initializeListElement(metadata_, waypoints_[0]);
  listElement[1] = initializeListElement(metadata_, waypoints_[1]);
  listElement[2] = initializeListElement(metadata_, waypoints_[2]);
  listElement[0].nextElementID(listElement[1].elementID());
  listElement[1].nextElementID(listElement[2].elementID());
  EXPECT_TRUE(list_->receive(listElement[0]));
  EXPECT_TRUE(list_->receive(listElement[1]));
  EXPECT_TRUE(list_->receive(listElement[2]));
  auto result = list_->receive(metadata_);
  EXPECT_EQ(result.status, arlcore::umaa::LargeListStatus::VALID_LIST);
  if (auto list = result.list.lock()) {
    EXPECT_EQ(list->size(), 3);
    auto it = list->begin();
    EXPECT_EQ(*it, waypoints_[0]);
    it = std::next(it);
    EXPECT_EQ(*it, waypoints_[1]);
    it = std::next(it);
    EXPECT_EQ(*it, waypoints_[2]);
  } else {
    FAIL() << "Unable to acquire lock";
  }

  listElement[0].nextElementID(listElement[2].elementID());
  listElement[0].elementTimestamp(DateTime(metadata_.updateElementTimestamp().value().seconds() + 1, 0));
  metadata_.updateElementID(listElement[1].elementID());
  metadata_.updateElementTimestamp().reset();
  metadata_.size(metadata_.size() - 1);
  EXPECT_TRUE(list_->receive(listElement[0]));
  EXPECT_TRUE(list_->dispose(listElement[1]));
  result = list_->receive(metadata_);
  EXPECT_EQ(result.status, arlcore::umaa::LargeListStatus::VALID_LIST);
  if (auto list = result.list.lock()) {
    EXPECT_EQ(list->size(), 2);
    auto it = list->begin();
    EXPECT_EQ(*it, waypoints_[0]);
    it = std::next(it);
    EXPECT_EQ(*it, waypoints_[2]);
  } else {
    FAIL() << "Unable to acquire lock";
  }
}

TEST_F(LargeListTest, listCache) {
  WaypointListElement listElement[3];
  listElement[0] = initializeListElement(metadata_, waypoints_[0]);
  listElement[1] = initializeListElement(metadata_, waypoints_[1]);
  listElement[2] = initializeListElement(metadata_, waypoints_[2]);
  listElement[0].nextElementID(listElement[1].elementID());
  listElement[1].nextElementID(listElement[2].elementID());
  list_->receive(listElement[0]);
  list_->receive(listElement[1]);
  list_->receive(listElement[2]);
  list_->receive(metadata_);
  
  // Make changes to the list after latest "commit"
  listElement[0].nextElementID(listElement[2].elementID());
  listElement[0].elementTimestamp(DateTime(metadata_.updateElementTimestamp().value().seconds() + 1, 0));
  metadata_.updateElementID(listElement[1].elementID());
  metadata_.updateElementTimestamp().reset();
  metadata_.size(metadata_.size() - 1);
  list_->receive(listElement[0]);
  list_->dispose(listElement[1]);
  
  // Should still be original list
  auto result = list_->getList();
  EXPECT_EQ(result.status, arlcore::umaa::LargeListStatus::VALID_LIST);
  if (auto list = result.list.lock()) {
    EXPECT_EQ(list->size(), 3);
    auto it = list->begin();
    EXPECT_EQ(*it, waypoints_[0]);
    it = std::next(it);
    EXPECT_EQ(*it, waypoints_[1]);
    it = std::next(it);
    EXPECT_EQ(*it, waypoints_[2]);
  } else {
    FAIL() << "Unable to acquire lock";
  }

  // Now when we update the metadata, we get the new list
  result = list_->receive(metadata_);
  EXPECT_EQ(result.status, arlcore::umaa::LargeListStatus::VALID_LIST);
  if (auto list = result.list.lock()) {
    EXPECT_EQ(list->size(), 2);
    auto it = list->begin();
    EXPECT_EQ(*it, waypoints_[0]);
    it = std::next(it);
    EXPECT_EQ(*it, waypoints_[2]);
  } else {
    FAIL() << "Unable to acquire lock";
  }
}