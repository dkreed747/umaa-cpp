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

#include "LargeListWriter.h"

#include <gtest/gtest.h>

#include <UMAA/MO/GlobalWaypointControl/GlobalWaypointCommandType.hpp>
#include <dds/dds.hpp>

#include "CycloneReader.h"
#include "CycloneSender.h"
#include "CycloneUtilities.h"
#include "UuidFactory.h"

using WaypointListElement = UMAA::MO::GlobalWaypointControl::GlobalWaypointCommandTypeWaypointsListElement;
using UMAA::MO::GlobalWaypointControl::GlobalWaypointCommandTypeTopic;
using UMAA::MO::GlobalWaypointControl::GlobalWaypointCommandType; 
using UMAA::MO::GlobalWaypointControl::GlobalWaypointCommandTypeWaypointsListElementTopic;
using UMAA::MO::GlobalWaypointControl::GlobalWaypointType;

class LargeListWriterTest : public ::testing::Test {
 protected:
  static void SetUpTestSuite() {
    // DDS Setup
    const int32_t domainId = 82;
    participant_ = arlcore::io::getDomainParticipant(domainId);

    // Topics
    const std::string listTopic = GlobalWaypointCommandTypeWaypointsListElementTopic;
    const std::string commandTopic = GlobalWaypointCommandTypeTopic;
    const auto ddsListTopic = arlcore::io::getTopic<WaypointListElement>(participant_, listTopic);

    // Publisher and Subscriber
    const auto ddsSub = arlcore::io::createSubscriber(participant_);
    const auto ddsPub = arlcore::io::createPublisher(participant_);

    // QoS Settings
    dds::sub::qos::DataReaderQos rQos;
    rQos << dds::core::policy::Reliability::Reliable()
         << dds::core::policy::History::KeepAll()
         << dds::core::policy::Durability::TransientLocal()
         << dds::core::policy::DestinationOrder::SourceTimestamp();
    
    dds::pub::qos::DataWriterQos wQos;
    wQos << dds::core::policy::Reliability::Reliable()
         << dds::core::policy::History::KeepAll()
         << dds::core::policy::Durability::TransientLocal()
         << dds::core::policy::DestinationOrder::SourceTimestamp();

    // Large List Writer
    listIo_ = std::make_shared<arlcore::io::CycloneSender<WaypointListElement>>(
        participant_, listTopic, wQos);

    listWriter_ = std::make_unique<arlcore::umaa::LargeListWriter<GlobalWaypointType, WaypointListElement>>(listIo_);

    // Test IO
    testReader_ = dds::sub::DataReader<WaypointListElement>(
      ddsSub, ddsListTopic, rQos);

    // Test Waypoints
    waypoints_[0] = initializeWaypoint(1.2, 12.345, -12.345, 2.345);
    waypoints_[1] = initializeWaypoint(2.2, 13.345, -13.345, 3.345);
    waypoints_[2] = initializeWaypoint(3.2, 14.345, -14.345, 4.345);
    waypoints_[3] = initializeWaypoint(4.2, 15.345, -15.345, 5.345);
    waypoints_[4] = initializeWaypoint(5.2, 16.345, -16.345, 6.345);

    usleep(10000);  // Discovery 0.01 seconds
  }
  void SetUp() override {
    // Empty test reader before the start of each TEST_F()
    testReader_.take();
  }

  void TearDown() override {
    // Clear Large List at the end of each TEST_F()
    listWriter_->clear();
    listIo_->waitForAcknowledgements(maxWait_);
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

    // Speed Control is also an optional...
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

  static dds::domain::DomainParticipant participant_;
  static std::shared_ptr<arlcore::io::CycloneSender<WaypointListElement>> listIo_;
  static dds::sub::DataReader<WaypointListElement> testReader_;
  static std::unique_ptr<arlcore::umaa::LargeListWriter<GlobalWaypointType, WaypointListElement>> listWriter_;
  const arlcore::io::Duration maxWait_ = {0, 1000000};  // 0.001 seconds
  static GlobalWaypointType waypoints_[5];
};

dds::domain::DomainParticipant LargeListWriterTest::participant_ = dds::core::null;
std::shared_ptr<arlcore::io::CycloneSender<WaypointListElement>> LargeListWriterTest::listIo_;
dds::sub::DataReader<WaypointListElement> LargeListWriterTest::testReader_ = dds::core::null;
std::unique_ptr<arlcore::umaa::LargeListWriter<GlobalWaypointType, WaypointListElement>>
  LargeListWriterTest::listWriter_;
GlobalWaypointType LargeListWriterTest::waypoints_[5];

TEST_F(LargeListWriterTest, emptyList) {
  auto metadata = listWriter_->getMetadata();
  EXPECT_EQ(metadata.updateElementID(), arlcore::NIL_GUID.getGuid());
  EXPECT_EQ(metadata.size(), 0);
  EXPECT_EQ(listWriter_->size(), 0);
  EXPECT_FALSE(metadata.updateElementTimestamp().has_value());
}

TEST_F(LargeListWriterTest, initialList) {
  std::vector<GlobalWaypointType> waypoints(waypoints_, waypoints_ + 2);
  arlcore::umaa::LargeListWriter<GlobalWaypointType, WaypointListElement> listWriter(listIo_, waypoints);
  auto metadata = listWriter.getMetadata();
  EXPECT_EQ(metadata.size(), 2);
  EXPECT_EQ(listWriter.size(), 2);
  EXPECT_EQ(listWriter.front().element(), waypoints[0]);
  EXPECT_EQ(listWriter.back().element(), waypoints[1]);
  listIo_->waitForAcknowledgements(maxWait_);
  

  auto samples = testReader_.take();
  dds::sub::LoanedSamples<WaypointListElement>::const_iterator sample = samples.begin();
  ASSERT_EQ(samples.length(), 2);
  EXPECT_EQ(sample[0].data(), listWriter.back());
  EXPECT_EQ(sample[0].data().listID(), listWriter.getListId().getGuid());
  EXPECT_FALSE(sample[0].data().nextElementID().has_value());

  EXPECT_EQ(sample[1].data(), listWriter.front());
  EXPECT_EQ(sample[1].data().listID(), listWriter.getListId().getGuid());
  EXPECT_EQ(sample[1].data().nextElementID().value(), sample[0].data().elementID());
  EXPECT_EQ(sample[1].data().elementTimestamp(), metadata.updateElementTimestamp().value());
  EXPECT_EQ(sample[1].data().elementID(), metadata.updateElementID());
}

TEST_F(LargeListWriterTest, elementAt) {
  std::vector<GlobalWaypointType> waypoints(waypoints_, waypoints_ + 2);
  EXPECT_EQ(waypoints.size(), 2);
  EXPECT_EQ(listWriter_->prepend(waypoints), arlcore::io::SendStatus::SUCCESS);
  auto metadata = listWriter_->getMetadata();
  EXPECT_EQ(metadata.size(), 2);
  EXPECT_EQ(listWriter_->size(), 2);
  EXPECT_EQ(listWriter_->front().element(), waypoints_[0]);
  EXPECT_EQ(listWriter_->back().element(), waypoints_[1]);
  listIo_->waitForAcknowledgements(maxWait_);
  
  auto samples = testReader_.take();
  dds::sub::LoanedSamples<WaypointListElement>::const_iterator sample = samples.begin();
  ASSERT_EQ(samples.length(), 2);
  // Check data
  EXPECT_EQ(sample[0].data().element(), waypoints_[1]);
  EXPECT_EQ(sample[1].data().element(), waypoints_[0]);

  EXPECT_EQ(listWriter_->elementAt(0), waypoints_[0]);
  EXPECT_EQ(listWriter_->elementAt(1), waypoints_[1]);
}

TEST_F(LargeListWriterTest, prependVector) {
  EXPECT_EQ(listWriter_->prepend(waypoints_[4]), arlcore::io::SendStatus::SUCCESS);
  auto metadata = listWriter_->getMetadata();
  EXPECT_EQ(metadata.size(), 1);
  EXPECT_EQ(listWriter_->size(), 1);
  EXPECT_EQ(listWriter_->front().element(), waypoints_[4]);
  listIo_->waitForAcknowledgements(maxWait_);

  auto samples = testReader_.take();
  dds::sub::LoanedSamples<WaypointListElement>::const_iterator sample = samples.begin();
  ASSERT_EQ(samples.length(), 1);
  EXPECT_EQ(sample[0].data().element(), waypoints_[4]);
  EXPECT_EQ(sample[0].data().listID(), listWriter_->getListId().getGuid());
  EXPECT_EQ(sample[0].data().elementID(), metadata.startingElementID());
  EXPECT_EQ(sample[0].data().elementTimestamp(), metadata.updateElementTimestamp().value());
  EXPECT_EQ(sample[0].data().elementID(), metadata.updateElementID());

  std::vector<GlobalWaypointType> waypoints(waypoints_, waypoints_ + 4);
  ASSERT_EQ(waypoints.size(), 4);
  auto oldStart = metadata.startingElementID();
 
  EXPECT_EQ(listWriter_->prepend(waypoints), arlcore::io::SendStatus::SUCCESS);
  metadata = listWriter_->getMetadata();
  EXPECT_EQ(metadata.size(), 5);
  listIo_->waitForAcknowledgements(maxWait_);

  samples = testReader_.take();
  sample = samples.begin();
  ASSERT_EQ(samples.length(), 4);

  // Check data
  EXPECT_EQ(sample[0].data().element(), waypoints_[3]);
  EXPECT_EQ(sample[1].data().element(), waypoints_[2]);
  EXPECT_EQ(sample[2].data().element(), waypoints_[1]);
  EXPECT_EQ(sample[3].data().element(), waypoints_[0]);

  // Check order
  EXPECT_EQ(sample[3].data().nextElementID().value(), sample[2].data().elementID());
  EXPECT_EQ(sample[2].data().nextElementID().value(), sample[1].data().elementID());
  EXPECT_EQ(sample[1].data().nextElementID().value(), sample[0].data().elementID());
  EXPECT_EQ(sample[0].data().nextElementID().value(), oldStart);

  EXPECT_EQ(sample[3].data().elementTimestamp(), metadata.updateElementTimestamp().value());
  EXPECT_EQ(sample[3].data().elementID(), metadata.updateElementID());
}

TEST_F(LargeListWriterTest, appendElements) {
  EXPECT_EQ(listWriter_->append(waypoints_[0]), arlcore::io::SendStatus::SUCCESS);
  auto metadata = listWriter_->getMetadata();
  EXPECT_EQ(metadata.size(), 1);
  EXPECT_EQ(listWriter_->size(), 1);
  EXPECT_EQ(listWriter_->front().element(), waypoints_[0]);
  listIo_->waitForAcknowledgements(maxWait_);

  auto samples = testReader_.take();
  dds::sub::LoanedSamples<WaypointListElement>::const_iterator sample = samples.begin();
  ASSERT_EQ(samples.length(), 1);
  EXPECT_EQ(sample[0].data().element(), waypoints_[0]);
  EXPECT_EQ(sample[0].data().listID(), listWriter_->getListId().getGuid());
  EXPECT_EQ(sample[0].data().elementID(), metadata.startingElementID());
  EXPECT_EQ(sample[0].data().elementTimestamp(), metadata.updateElementTimestamp().value());
  EXPECT_EQ(sample[0].data().elementID(), metadata.updateElementID());

  EXPECT_EQ(listWriter_->append(waypoints_[1]), arlcore::io::SendStatus::SUCCESS);
  metadata = listWriter_->getMetadata();
  EXPECT_EQ(metadata.size(), 2);
  listIo_->waitForAcknowledgements(maxWait_);

  samples = testReader_.take();
  sample = samples.begin();
  ASSERT_EQ(samples.length(), 2);
  EXPECT_EQ(sample[0].data().element(), waypoints_[0]);
  EXPECT_EQ(sample[1].data().element(), waypoints_[1]);
  EXPECT_EQ(sample[0].data().listID(), listWriter_->getListId().getGuid());
  EXPECT_EQ(sample[1].data().listID(), listWriter_->getListId().getGuid());
  EXPECT_EQ(sample[0].data().nextElementID().value(), sample[1].data().elementID());
  EXPECT_FALSE(sample[1].data().nextElementID().has_value());
  EXPECT_EQ(sample[1].data().elementTimestamp(), metadata.updateElementTimestamp().value());
  EXPECT_EQ(sample[1].data().elementID(), metadata.updateElementID());
}

TEST_F(LargeListWriterTest, appendVector) {
  EXPECT_EQ(listWriter_->append(waypoints_[0]), arlcore::io::SendStatus::SUCCESS);
  auto metadata = listWriter_->getMetadata();
  EXPECT_EQ(metadata.size(), 1);
  EXPECT_EQ(listWriter_->size(), 1);
  EXPECT_EQ(listWriter_->front().element(), waypoints_[0]);
  listIo_->waitForAcknowledgements(maxWait_);

  auto samples = testReader_.take();
  dds::sub::LoanedSamples<WaypointListElement>::const_iterator sample = samples.begin();
  ASSERT_EQ(samples.length(), 1);
  EXPECT_EQ(sample[0].data().element(), waypoints_[0]);
  EXPECT_EQ(sample[0].data().listID(), listWriter_->getListId().getGuid());
  EXPECT_EQ(sample[0].data().elementID(), metadata.startingElementID());
  EXPECT_EQ(sample[0].data().elementTimestamp(), metadata.updateElementTimestamp().value());
  EXPECT_EQ(sample[0].data().elementID(), metadata.updateElementID());

  auto wpts = std::vector<GlobalWaypointType>(std::begin(waypoints_) + 1, std::end(waypoints_));
  EXPECT_EQ(listWriter_->append(wpts), arlcore::io::SendStatus::SUCCESS);
  metadata = listWriter_->getMetadata();
  EXPECT_EQ(metadata.size(), 5);
  listIo_->waitForAcknowledgements(maxWait_);

  samples = testReader_.take();
  sample = samples.begin();
  ASSERT_EQ(samples.length(), 5);

  // Check data
  EXPECT_EQ(sample[0].data().element(), waypoints_[0]);
  EXPECT_EQ(sample[1].data().element(), waypoints_[1]);
  EXPECT_EQ(sample[2].data().element(), waypoints_[2]);
  EXPECT_EQ(sample[3].data().element(), waypoints_[3]);
  EXPECT_EQ(sample[4].data().element(), waypoints_[4]);

  // Check List ID
  EXPECT_EQ(sample[0].data().listID(), listWriter_->getListId().getGuid());
  EXPECT_EQ(sample[1].data().listID(), listWriter_->getListId().getGuid());
  EXPECT_EQ(sample[2].data().listID(), listWriter_->getListId().getGuid());
  EXPECT_EQ(sample[3].data().listID(), listWriter_->getListId().getGuid());
  EXPECT_EQ(sample[4].data().listID(), listWriter_->getListId().getGuid());

  // Check order
  EXPECT_EQ(sample[0].data().nextElementID().value(), sample[1].data().elementID());
  EXPECT_EQ(sample[1].data().nextElementID().value(), sample[2].data().elementID());
  EXPECT_EQ(sample[2].data().nextElementID().value(), sample[3].data().elementID());
  EXPECT_EQ(sample[3].data().nextElementID().value(), sample[4].data().elementID());
  EXPECT_FALSE(sample[4].data().nextElementID().has_value());

  EXPECT_EQ(sample[4].data().elementTimestamp(), metadata.updateElementTimestamp().value());
  EXPECT_EQ(sample[4].data().elementID(), metadata.updateElementID());
}

TEST_F(LargeListWriterTest, popBack) {
  EXPECT_EQ(listWriter_->prepend(waypoints_[1]), arlcore::io::SendStatus::SUCCESS);
  EXPECT_EQ(listWriter_->prepend(waypoints_[0]), arlcore::io::SendStatus::SUCCESS);
  auto metadata = listWriter_->getMetadata();
  EXPECT_EQ(metadata.size(), 2);
  listIo_->waitForAcknowledgements(maxWait_);

  auto samples = testReader_.take();
  ASSERT_EQ(samples.length(), 2);
  auto sample = samples.begin();
  EXPECT_EQ(sample[0].data().element(), waypoints_[1]);
  EXPECT_EQ(sample[0].data().listID(), listWriter_->getListId().getGuid());
  auto wp1_id = sample[0].data().elementID();
  EXPECT_EQ(sample[1].data().element(), waypoints_[0]);
  EXPECT_EQ(sample[0].data().elementID(), sample[1].data().nextElementID().value());

  EXPECT_EQ(listWriter_->popBack(), arlcore::io::SendStatus::SUCCESS);
  metadata = listWriter_->getMetadata();
  EXPECT_EQ(metadata.size(), 1);
  EXPECT_FALSE(metadata.updateElementTimestamp().has_value());
  listIo_->waitForAcknowledgements(maxWait_);

  samples = testReader_.take();
  sample = samples.begin();
  // Receive one sample of the disposed waypoint
  ASSERT_EQ(samples.length(), 1);
  EXPECT_EQ(sample[0].data().element(), waypoints_[1]);
  EXPECT_FALSE(sample[0].data().nextElementID().has_value());
  EXPECT_EQ(sample[0].info().state().instance_state(), dds::sub::status::InstanceState::not_alive_disposed());
  WaypointListElement keySample;
  testReader_.key_value(keySample, sample[0].info().instance_handle());
  EXPECT_EQ(wp1_id, keySample.elementID());
  EXPECT_EQ(keySample.elementID(), metadata.updateElementID());
  EXPECT_FALSE(metadata.updateElementTimestamp().has_value());
}

TEST_F(LargeListWriterTest, popFront) {
  EXPECT_EQ(listWriter_->append(waypoints_[0]), arlcore::io::SendStatus::SUCCESS);
  EXPECT_EQ(listWriter_->popFront(), arlcore::io::SendStatus::SUCCESS);
  listIo_->waitForAcknowledgements(maxWait_);
  usleep(20000);

  auto metadata = listWriter_->getMetadata();
  EXPECT_EQ(metadata.size(), 0);
  auto samples = testReader_.take();
  auto sample = samples.begin();
  ASSERT_EQ(samples.length(), 1);
  EXPECT_EQ(sample[0].data().element(), waypoints_[0]);
  EXPECT_EQ(sample[0].data().listID(), listWriter_->getListId().getGuid());
  EXPECT_EQ(sample[0].info().state().instance_state(), dds::sub::status::InstanceState::not_alive_disposed());
  WaypointListElement keySample;
  testReader_.key_value(keySample, sample[0].info().instance_handle());
  EXPECT_EQ(sample[0].data().elementID(), keySample.elementID());
  EXPECT_EQ(keySample.elementID(), metadata.updateElementID());
  EXPECT_FALSE(metadata.updateElementTimestamp().has_value());
}

TEST_F(LargeListWriterTest, clear) {
  EXPECT_EQ(listWriter_->prepend(waypoints_[1]), arlcore::io::SendStatus::SUCCESS);
  EXPECT_EQ(listWriter_->prepend(waypoints_[0]), arlcore::io::SendStatus::SUCCESS);
  listIo_->waitForAcknowledgements(maxWait_);

  auto samples = testReader_.take();
  auto sample = samples.begin();
  ASSERT_EQ(samples.length(), 2);
  const UMAA::Common::Measurement::NumericGUID ids[] = {sample[0].data().elementID(), sample[1].data().elementID()};

  listWriter_->clear();
  auto metadata = listWriter_->getMetadata();
  EXPECT_EQ(metadata.size(), 0);
  EXPECT_FALSE(metadata.updateElementTimestamp().has_value());
  listIo_->waitForAcknowledgements(maxWait_);

  samples = testReader_.take();
  ASSERT_EQ(samples.length(), 2);
  WaypointListElement keySample0, keySample1;
  testReader_.key_value(keySample0, sample[0].info().instance_handle());
  testReader_.key_value(keySample1, sample[1].info().instance_handle());
  EXPECT_EQ(keySample0.elementID(), ids[0]);
  EXPECT_EQ(keySample1.elementID(), ids[1]);
}

TEST_F(LargeListWriterTest, insert) {
  EXPECT_EQ(listWriter_->prepend(waypoints_[1]), arlcore::io::SendStatus::SUCCESS);
  EXPECT_EQ(listWriter_->prepend(waypoints_[0]), arlcore::io::SendStatus::SUCCESS);
  EXPECT_EQ(listWriter_->insert(waypoints_[2], 1), arlcore::io::SendStatus::SUCCESS);
  auto metadata = listWriter_->getMetadata();
  listIo_->waitForAcknowledgements(maxWait_);

  auto samples = testReader_.take();
  auto sample = samples.begin();
  ASSERT_EQ(samples.length(), 4);
  EXPECT_EQ(sample[0].data().element(), waypoints_[1]);
  EXPECT_EQ(sample[1].data().element(), waypoints_[0]);
  EXPECT_EQ(sample[1].data().nextElementID().value(), sample[0].data().elementID());
  EXPECT_EQ(sample[2].data().element(), waypoints_[0]);
  EXPECT_EQ(sample[2].data().nextElementID().value(), sample[3].data().elementID());
  EXPECT_EQ(sample[3].data().element(), waypoints_[2]);
  EXPECT_EQ(sample[3].data().elementID(), metadata.updateElementID());
  EXPECT_EQ(sample[3].data().elementTimestamp(), metadata.updateElementTimestamp().value());
}

TEST_F(LargeListWriterTest, insertVector) {
  EXPECT_EQ(listWriter_->prepend(waypoints_[4]), arlcore::io::SendStatus::SUCCESS);
  EXPECT_EQ(listWriter_->prepend(waypoints_[0]), arlcore::io::SendStatus::SUCCESS);
  auto metadata = listWriter_->getMetadata();
  EXPECT_EQ(metadata.size(), 2);
  EXPECT_EQ(listWriter_->size(), 2);
  EXPECT_EQ(listWriter_->front().element(), waypoints_[0]);
  EXPECT_EQ(listWriter_->back().element(), waypoints_[4]);
  listIo_->waitForAcknowledgements(maxWait_);

  auto samples = testReader_.take();
  auto sample = samples.begin();
  ASSERT_EQ(samples.length(), 2);
  EXPECT_EQ(sample[0].data().element(), waypoints_[4]);
  EXPECT_EQ(sample[0].data().listID(), listWriter_->getListId().getGuid());
  auto lastElementId = sample[0].data().elementID();

  EXPECT_EQ(sample[1].data().element(), waypoints_[0]);
  EXPECT_EQ(sample[1].data().elementID(), metadata.startingElementID());
  EXPECT_EQ(sample[1].data().nextElementID().value(), lastElementId);
  EXPECT_EQ(sample[1].data().elementTimestamp(), metadata.updateElementTimestamp().value());
  EXPECT_EQ(sample[1].data().elementID(), metadata.updateElementID());

  std::vector<GlobalWaypointType> wpts(waypoints_ + 1, waypoints_ + 4);
  EXPECT_EQ(listWriter_->insert(wpts, 1), arlcore::io::SendStatus::SUCCESS);
  metadata = listWriter_->getMetadata();

  listIo_->waitForAcknowledgements(maxWait_);

  samples = testReader_.take();
  sample = samples.begin();
  ASSERT_EQ(samples.length(), 4);

  // Check data
  EXPECT_EQ(sample[0].data().element(), waypoints_[0]);
  EXPECT_EQ(sample[1].data().element(), waypoints_[1]);
  EXPECT_EQ(sample[2].data().element(), waypoints_[2]);
  EXPECT_EQ(sample[3].data().element(), waypoints_[3]);

  // Check order
  EXPECT_EQ(sample[0].data().nextElementID().value(), sample[1].data().elementID());
  EXPECT_EQ(sample[1].data().nextElementID().value(), sample[2].data().elementID());
  EXPECT_EQ(sample[2].data().nextElementID().value(), sample[3].data().elementID());
  EXPECT_EQ(sample[3].data().nextElementID().value(), lastElementId);
  
  EXPECT_EQ(sample[3].data().elementID(), metadata.updateElementID());
  EXPECT_EQ(sample[3].data().elementTimestamp(), metadata.updateElementTimestamp().value());
}

TEST_F(LargeListWriterTest, update) {
  EXPECT_EQ(listWriter_->append(waypoints_[0]), arlcore::io::SendStatus::SUCCESS);
  EXPECT_EQ(listWriter_->update(waypoints_[1], 0), arlcore::io::SendStatus::SUCCESS);
  auto metadata = listWriter_->getMetadata();
  listIo_->waitForAcknowledgements(maxWait_);
  auto samples = testReader_.take();
  auto sample = samples.begin();
  ASSERT_EQ(samples.length(), 2);
  EXPECT_EQ(sample[0].data().element(), waypoints_[0]);
  EXPECT_EQ(sample[1].data().element(), waypoints_[1]);
  EXPECT_EQ(sample[0].data().elementID(), sample[1].data().elementID());
  EXPECT_EQ(sample[1].data().elementID(), metadata.updateElementID());
  EXPECT_EQ(sample[1].data().elementTimestamp(), metadata.updateElementTimestamp());
}

TEST_F(LargeListWriterTest, remove) {
  EXPECT_EQ(listWriter_->prepend(waypoints_[2]), arlcore::io::SendStatus::SUCCESS); // Sample 0
  EXPECT_EQ(listWriter_->prepend(waypoints_[1]), arlcore::io::SendStatus::SUCCESS); // Sample 1
  EXPECT_EQ(listWriter_->prepend(waypoints_[0]), arlcore::io::SendStatus::SUCCESS); // Sample 2
  listIo_->waitForAcknowledgements(maxWait_);

  auto samples = testReader_.take();
  auto sample = samples.begin();

  ASSERT_EQ(samples.length(), 3);
  EXPECT_EQ(sample[0].data().element(), waypoints_[2]);
  EXPECT_EQ(sample[1].data().element(), waypoints_[1]);
  EXPECT_EQ(sample[2].data().element(), waypoints_[0]);
  EXPECT_EQ(sample[1].data().nextElementID().value(), sample[0].data().elementID());
  EXPECT_EQ(sample[2].data().nextElementID().value(), sample[1].data().elementID());
  auto updateId = sample[2].data().elementID();
  auto nextId = sample[0].data().elementID();

  EXPECT_EQ(listWriter_->remove(1), arlcore::io::SendStatus::SUCCESS);
  auto metadata = listWriter_->getMetadata();
  EXPECT_FALSE(metadata.updateElementTimestamp().has_value());
  listIo_->waitForAcknowledgements(maxWait_);

  samples = testReader_.take();
  sample = samples.begin();
  ASSERT_EQ(samples.length(), 2);
  EXPECT_EQ(sample[0].data().elementID(), updateId);
  EXPECT_EQ(sample[0].data().nextElementID().value(), nextId);
  EXPECT_EQ(sample[1].info().state().instance_state(), dds::sub::status::InstanceState::not_alive_disposed());
  WaypointListElement keySample;
  testReader_.key_value(keySample, sample[1].info().instance_handle());
  EXPECT_EQ(keySample.elementID(), metadata.updateElementID());
}