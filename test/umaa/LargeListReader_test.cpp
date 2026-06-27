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

#include "LargeListReader.h"

#include <gtest/gtest.h>

#include <UMAA/MO/GlobalWaypointControl/GlobalWaypointCommandType.hpp>

#include "LocalReaderSender.h"
#include "UuidFactory.h"

using WaypointListElement = UMAA::MO::GlobalWaypointControl::GlobalWaypointCommandTypeWaypointsListElement;
using UMAA::Common::LargeListMetadata;
using UMAA::Common::Measurement::DateTime;
using UMAA::MO::GlobalWaypointControl::GlobalWaypointCommandTypeTopic;
using UMAA::MO::GlobalWaypointControl::GlobalWaypointCommandType;
using UMAA::MO::GlobalWaypointControl::GlobalWaypointCommandTypeWaypointsListElementTopic;
using UMAA::MO::GlobalWaypointControl::GlobalWaypointType;

class LargeListReaderTest : public ::testing::Test {
 protected:
  static void SetUpTestSuite() {
    reader_ = std::make_shared<arlcore::io::LocalReaderSender<WaypointListElement>>();
    waypoints_[0] = initializeWaypoint(1.2, 12.345, -12.345, 2.345);
    waypoints_[1] = initializeWaypoint(2.2, 13.345, -13.345, 3.345);
    waypoints_[2] = initializeWaypoint(3.2, 14.345, -14.345, 4.345);
    waypoints_[3] = initializeWaypoint(4.2, 15.345, -15.345, 5.345);
    waypoints_[4] = initializeWaypoint(5.2, 16.345, -16.345, 6.345);
  }
  void SetUp() override {
    listReader_ = std::make_unique<arlcore::umaa::LargeListReader<GlobalWaypointType, WaypointListElement>>(reader_);
  }

  static const std::string listTopic_;
  static const std::string commandTopic_;
  static std::shared_ptr<arlcore::io::LocalReaderSender<WaypointListElement>> reader_;
  std::unique_ptr<arlcore::umaa::LargeListReader<GlobalWaypointType, WaypointListElement>> listReader_;
  static GlobalWaypointType waypoints_[5];

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

  static GlobalWaypointType initializeWaypoint(flt64_t depth, flt64_t lat, flt64_t lon, flt64_t speed) {
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

  LargeListMetadata initializeMetadata() {
    LargeListMetadata metadata;
    metadata.listID(arlcore::UuidFactory::getInstance().generateGuid().getGuid());
    metadata.updateElementID(arlcore::NIL_GUID.getGuid());
    metadata.size(0);
    return metadata;
  }
};

const std::string LargeListReaderTest::listTopic_ = GlobalWaypointCommandTypeWaypointsListElementTopic;
const std::string LargeListReaderTest::commandTopic_ = GlobalWaypointCommandTypeTopic;
std::shared_ptr<arlcore::io::LocalReaderSender<WaypointListElement>> LargeListReaderTest::reader_(nullptr);
GlobalWaypointType LargeListReaderTest::waypoints_[5];

TEST_F(LargeListReaderTest, emptyList) {
  auto metadata = initializeMetadata();
  EXPECT_EQ(listReader_->getListFromMetadata(metadata).status, arlcore::umaa::LargeListStatus::EMPTY_LIST);
}

TEST_F(LargeListReaderTest, disposeList) {
  auto metadata = initializeMetadata();
  WaypointListElement e = initializeListElement(metadata, waypoints_[0]);
  EXPECT_EQ(reader_->send(e), arlcore::io::SendStatus::SUCCESS);

  auto result = listReader_->getListFromMetadata(metadata);
  EXPECT_EQ(result.status, arlcore::umaa::LargeListStatus::VALID_LIST);
  EXPECT_FALSE(result.list.expired());
  if (auto list = result.list.lock()) {
    EXPECT_EQ(list->size(), 1);
    EXPECT_EQ(list->front(), waypoints_[0]);
  } else {
    FAIL() << "Unable to acquire lock";
  }
  EXPECT_EQ(reader_->dispose(e), arlcore::io::SendStatus::SUCCESS);

  EXPECT_TRUE(listReader_->removeListByMetadata(metadata));
  EXPECT_TRUE(result.list.expired());
  result = listReader_->getListById(arlcore::NumericGuid(metadata.listID()));
  EXPECT_EQ(result.status, arlcore::umaa::LargeListStatus::INVALID_LIST);
}

TEST_F(LargeListReaderTest, multiElementList) {
  auto metadata = initializeMetadata();
  WaypointListElement listElement[3];
  listElement[0] = initializeListElement(metadata, waypoints_[0]);
  listElement[1] = initializeListElement(metadata, waypoints_[1]);
  listElement[2] = initializeListElement(metadata, waypoints_[2]);
  listElement[0].nextElementID(listElement[1].elementID());
  listElement[1].nextElementID(listElement[2].elementID());
  reader_->send(listElement[0]);
  reader_->send(listElement[1]);
  reader_->send(listElement[2]);


  auto result = listReader_->getListFromMetadata(metadata);
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

TEST_F(LargeListReaderTest, multiList) {
  // Initialize data for 2 separate lists
  auto metadata_0 = initializeMetadata();
  auto metadata_1 = initializeMetadata();
  WaypointListElement listElement[2][3];
  listElement[0][0] = initializeListElement(metadata_0, waypoints_[0]);
  listElement[0][1] = initializeListElement(metadata_0, waypoints_[1]);
  listElement[0][2] = initializeListElement(metadata_0, waypoints_[2]);
  listElement[0][0].nextElementID(listElement[0][1].elementID());
  listElement[0][1].nextElementID(listElement[0][2].elementID());
  listElement[1][0] = initializeListElement(metadata_1, waypoints_[2]);
  listElement[1][1] = initializeListElement(metadata_1, waypoints_[3]);
  listElement[1][2] = initializeListElement(metadata_1, waypoints_[4]);
  listElement[1][0].nextElementID(listElement[1][1].elementID());
  listElement[1][1].nextElementID(listElement[1][2].elementID());

  // Send elements of both lists, intermixed and out of order
  reader_->send(listElement[0][2]);
  reader_->send(listElement[1][1]);
  reader_->send(listElement[0][1]);
  reader_->send(listElement[1][0]);
  reader_->send(listElement[1][2]);
  reader_->send(listElement[0][0]);

  auto result = listReader_->getListFromMetadata(metadata_0);
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

  result = listReader_->getListFromMetadata(metadata_1);
  EXPECT_EQ(result.status, arlcore::umaa::LargeListStatus::VALID_LIST);
  if (auto list = result.list.lock()) {
    EXPECT_EQ(list->size(), 3);
    auto it = list->begin();
    EXPECT_EQ(*it, waypoints_[2]);
    it = std::next(it);
    EXPECT_EQ(*it, waypoints_[3]);
    it = std::next(it);
    EXPECT_EQ(*it, waypoints_[4]);
  } else {
    FAIL() << "Unable to acquire lock";
  }
}

TEST_F(LargeListReaderTest, disposeListElement) {
  auto metadata = initializeMetadata();
  WaypointListElement listElement[3];
  listElement[0] = initializeListElement(metadata, waypoints_[0]);
  listElement[1] = initializeListElement(metadata, waypoints_[1]);
  listElement[2] = initializeListElement(metadata, waypoints_[2]);
  listElement[0].nextElementID(listElement[1].elementID());
  listElement[1].nextElementID(listElement[2].elementID());
  reader_->send(listElement[0]);
  reader_->send(listElement[1]);
  reader_->send(listElement[2]);

  auto result = listReader_->getListFromMetadata(metadata);
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
  listElement[0].elementTimestamp(DateTime(metadata.updateElementTimestamp().value().seconds() + 1, 0));
  metadata.updateElementID(listElement[1].elementID());
  metadata.updateElementTimestamp().reset();
  metadata.size(metadata.size() - 1);
  reader_->send(listElement[0]);
  reader_->dispose(listElement[1]);

  result = listReader_->getListFromMetadata(metadata);
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

TEST_F(LargeListReaderTest, getListById) {
  auto metadata = initializeMetadata();
  WaypointListElement listElement[3];
  listElement[0] = initializeListElement(metadata, waypoints_[0]);
  listElement[1] = initializeListElement(metadata, waypoints_[1]);
  listElement[2] = initializeListElement(metadata, waypoints_[2]);
  listElement[0].nextElementID(listElement[1].elementID());
  listElement[1].nextElementID(listElement[2].elementID());
  reader_->send(listElement[0]);
  reader_->send(listElement[1]);
  reader_->send(listElement[2]);

  listReader_->getListFromMetadata(metadata);

  arlcore::NumericGuid listId(metadata.listID());
  auto result = listReader_->getListById(listId);
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

TEST_F(LargeListReaderTest, getListByBadId) {
  arlcore::NumericGuid listId = arlcore::NIL_GUID;
  auto result = listReader_->getListById(listId);
  EXPECT_EQ(result.status, arlcore::umaa::LargeListStatus::INVALID_LIST);
  EXPECT_TRUE(result.list.expired());
}