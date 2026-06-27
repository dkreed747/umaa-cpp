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

#include <UMAA/MM/BaseType/HoverObjectiveType.hpp>
#include <UMAA/MM/BaseType/RouteObjectiveType.hpp>

#include "DefaultSendableSpec.h"
#include "LargeListReader.h"
#include "LocalReaderSender.h"
#include "RouteSendableSpec.h"

using arlcore::io::LocalReaderSender;
using arlcore::io::ReaderBase;
using arlcore::io::ReadStatus;
using arlcore::io::SenderBase;
using arlcore::io::SendStatus;
using arlcore::umaa::DefaultSendableSpec;
using arlcore::umaa::LargeListReader;
using arlcore::umaa::LargeListStatus;
using arlcore::umaa::RouteSendableSpec;

class SendableObjSpecializationTest : public ::testing::Test {
 protected:
  SendableObjSpecializationTest()
      : hoverRS_(std::make_shared<LocalReaderSender<HoverObjectiveType>>()),
        routeRS_(std::make_shared<LocalReaderSender<RouteObjectiveType>>()),
        routeWptRS_(std::make_shared<LocalReaderSender<RouteObjectiveTypeWaypointsListElement>>()) {
    wptListReader_ =
        std::make_shared<LargeListReader<WaypointType, RouteObjectiveTypeWaypointsListElement>>(routeWptRS_);

    for (uint8_t i = 0; i < WPT_SIZE; ++i) {
      WaypointType wpt;
      wpt.captureRadius().distance(i);
      testWptList_.push_back(wpt);
    }
  }

  std::shared_ptr<LocalReaderSender<HoverObjectiveType>> hoverRS_;
  std::shared_ptr<LocalReaderSender<RouteObjectiveType>> routeRS_;
  std::shared_ptr<LocalReaderSender<RouteObjectiveTypeWaypointsListElement>> routeWptRS_;
  std::shared_ptr<LargeListReader<WaypointType, RouteObjectiveTypeWaypointsListElement>> wptListReader_;
  std::list<WaypointType> testWptList_;
  const uint8_t WPT_SIZE = 3;
  const NumericGUID TEST_SPECID_1 = arlcore::UuidFactory::getInstance().generateGuid().getGuid();
  const NumericGUID TEST_SPECID_2 = arlcore::UuidFactory::getInstance().generateGuid().getGuid();
};

TEST_F(SendableObjSpecializationTest, MakeAndSendDefault) {
  HoverObjectiveType spec;
  spec.duration(5);
  spec.specializationReferenceID(TEST_SPECID_1);

  DefaultSendableSpec<HoverObjectiveType> testSpec(spec, hoverRS_, HoverObjectiveTypeTopic);

  testSpec.send();
  EXPECT_EQ(testSpec.send(), SendStatus::ERROR) << "Expected second send call to fail.";
  testSpec.dispose();

  // Check that the send and disposed samples match the contents of DefaultSendableSpec
  HoverObjectiveType recvSpec;
  EXPECT_EQ(hoverRS_->read(&recvSpec), ReadStatus::SUCCESS);
  EXPECT_EQ(recvSpec, spec);
  EXPECT_EQ(hoverRS_->read(&recvSpec), ReadStatus::DISPOSED);
  EXPECT_EQ(recvSpec, spec);
}

TEST_F(SendableObjSpecializationTest, CopyDefault) {
  HoverObjectiveType spec;
  spec.duration(5);
  spec.specializationReferenceID(TEST_SPECID_1);

  DefaultSendableSpec<HoverObjectiveType> testSpec(spec, hoverRS_, HoverObjectiveTypeTopic);

  auto specCopy = std::dynamic_pointer_cast<DefaultSendableSpec<HoverObjectiveType>>(
      testSpec.with(TEST_SPECID_2, arlcore::umaa::getTimestamp()));

  EXPECT_EQ(specCopy->getSpecializationData().duration(), testSpec.getSpecializationData().duration());

  EXPECT_EQ(testSpec.getSpecializationID(), TEST_SPECID_1);
  EXPECT_EQ(specCopy->getSpecializationID(), TEST_SPECID_2);
}

TEST_F(SendableObjSpecializationTest, MakeAndSendRoute) {
  RouteObjectiveType spec;
  spec.specializationReferenceID(TEST_SPECID_1);

  RouteSendableSpec testSpec(spec, testWptList_, routeRS_, routeWptRS_);

  // No elements were sent out to the bus yet.
  EXPECT_EQ(testSpec.getSpecializationData().waypointsListMetadata().size(), 0);
  // SEND
  testSpec.send();

  EXPECT_EQ(testSpec.getSpecializationData().waypointsListMetadata().size(), WPT_SIZE);

  // Validate Route and Waypoints
  RouteObjectiveType recvSpec;
  ASSERT_EQ(routeRS_->read(&recvSpec), ReadStatus::SUCCESS);
  EXPECT_EQ(recvSpec.specializationReferenceID(), TEST_SPECID_1);

  auto recvWpts =
      wptListReader_->getListFromMetadata(testSpec.getSpecializationData().waypointsListMetadata()).list.lock();
  EXPECT_EQ(recvWpts->size(), WPT_SIZE);
  auto wptListIt = testWptList_.begin();
  for (auto it = recvWpts->begin(); it != recvWpts->end(); ++it, ++wptListIt) {
    EXPECT_EQ(it->captureRadius().distance(), wptListIt->captureRadius().distance());
  }

  // DISPOSE
  testSpec.dispose();
  RouteObjectiveType disposedSpec;
  ASSERT_EQ(routeRS_->read(&disposedSpec), ReadStatus::DISPOSED);
  EXPECT_EQ(recvSpec.specializationReferenceID(), TEST_SPECID_1);
  RouteObjectiveTypeWaypointsListElement elem;
  ReadStatus status = ReadStatus::SUCCESS;

  for (uint8_t i = 0; i < WPT_SIZE; ++i) {
    status = routeWptRS_->read(&elem);

    ASSERT_EQ(status, ReadStatus::DISPOSED);
  }

  status = routeWptRS_->read(&elem);
  ASSERT_EQ(status, ReadStatus::NO_DATA);
}

TEST_F(SendableObjSpecializationTest, CopyRoute) {
  RouteObjectiveType spec;
  spec.specializationReferenceID(TEST_SPECID_1);

  RouteSendableSpec testSpec(spec, testWptList_, routeRS_, routeWptRS_);

  auto specCopy =
      std::dynamic_pointer_cast<RouteSendableSpec>(testSpec.with(TEST_SPECID_2, arlcore::umaa::getTimestamp()));

  specCopy->send();  // Updates the copy's Large List Metadata

  EXPECT_NE(specCopy->getSpecializationData().waypointsListMetadata().listID(),
            testSpec.getSpecializationData().waypointsListMetadata().listID());

  EXPECT_EQ(testSpec.getSpecializationID(), TEST_SPECID_1);
  EXPECT_EQ(specCopy->getSpecializationID(), TEST_SPECID_2);
}
