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

#include "ObjectiveSpecFactory.h"

#include <gtest/gtest.h>

#include <memory>

#include "LocalReaderSender.h"
#include "MockReaderSender.h"
#include "UmaaUtils.h"
#include "UuidFactory.h"

using arlcore::io::IOReaderRegistry;
using arlcore::io::IOWriterRegistry;
using arlcore::io::LocalReaderSender;
using arlcore::io::ReaderBase;
using arlcore::io::ReadStatus;
using arlcore::io::SenderBase;
using arlcore::io::SendStatus;
using arlcore::test::MockReaderSender;
using arlcore::umaa::DefaultSendableSpec;
using arlcore::umaa::ObjectiveSpecFactory;
using arlcore::umaa::RouteSendableSpec;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::SetArgPointee;

class ObjectiveSpecFactory_test : public ::testing::Test {
 protected:
  ObjectiveSpecFactory_test()
      : hoverRS_(std::make_shared<LocalReaderSender<HoverObjectiveType>>()),
        routeRS_(std::make_shared<MockReaderSender<RouteObjectiveType>>()),
        routeElemRS_(std::make_shared<LocalReaderSender<RouteObjectiveTypeWaypointsListElement>>()),
        objRS_(std::make_shared<LocalReaderSender<ObjectiveType>>()),
        wRegistry_(std::make_shared<IOWriterRegistry>()),
        rRegistry_(std::make_shared<IOReaderRegistry>()),
        factory_(rRegistry_, wRegistry_) {
    wRegistry_->registerWriter<HoverObjectiveType>(HoverObjectiveTypeTopic, hoverRS_);
    rRegistry_->registerReader<HoverObjectiveType>(HoverObjectiveTypeTopic, hoverRS_);

    wRegistry_->registerWriter<RouteObjectiveType>(RouteObjectiveTypeTopic, routeRS_);
    rRegistry_->registerReader<RouteObjectiveType>(RouteObjectiveTypeTopic, routeRS_);

    wRegistry_->registerWriter<RouteObjectiveTypeWaypointsListElement>(RouteObjectiveTypeWaypointsListElementTopic,
                                                                       routeElemRS_);
    factory_.setWptLargeListReader(
        make_shared<LargeListReader<WaypointType, RouteObjectiveTypeWaypointsListElement>>(routeElemRS_));

    wptLLRS_ = make_shared<LargeListWriter<WaypointType, RouteObjectiveTypeWaypointsListElement>>(routeElemRS_);

    for (int i = 0; i < WAYPOINT_COUNT; i++) {
      WaypointType wpt;
      const arlcore::NumericGuid wptId = arlcore::UuidFactory::getInstance().generateGuid();
      wpt.position().geodeticLatitude(i * 10);
      wpt.position().geodeticLongitude(i * 5);
      wpt.waypointID(wptId.getGuid());
      waypoints_.push_back(wpt);
    }
  }

  std::shared_ptr<LocalReaderSender<HoverObjectiveType>> hoverRS_;
  std::shared_ptr<MockReaderSender<RouteObjectiveType>> routeRS_;
  std::shared_ptr<LocalReaderSender<RouteObjectiveTypeWaypointsListElement>> routeElemRS_;
  std::shared_ptr<LargeListWriter<WaypointType, RouteObjectiveTypeWaypointsListElement>> wptLLRS_;
  std::shared_ptr<LocalReaderSender<ObjectiveType>> objRS_;
  shared_ptr<IOWriterRegistry> wRegistry_;
  shared_ptr<IOReaderRegistry> rRegistry_;
  ObjectiveSpecFactory factory_;
  std::list<WaypointType> waypoints_;
  const int WAYPOINT_COUNT = 2;
  const NumericGUID TEST_SPECID_1 = arlcore::UuidFactory::getInstance().generateGuid().getGuid();
  const NumericGUID TEST_SPECID_2 = arlcore::UuidFactory::getInstance().generateGuid().getGuid();
};

template <typename T>
std::pair<ObjectiveType, T> makeObjective(const std::string& topic, const NumericGUID& specID) {
  auto ts = arlcore::umaa::getTimestamp();
  ObjectiveType obj;
  obj.specializationTopic(topic);
  obj.specializationID(specID);
  obj.specializationTimestamp(ts);

  T spec;
  spec.specializationReferenceID(specID);
  spec.specializationReferenceTimestamp(ts);
  return std::make_pair(obj, spec);
}

TEST_F(ObjectiveSpecFactory_test, MakeHover) {
  auto testObj = makeObjective<HoverObjectiveType>(HoverObjectiveTypeTopic, TEST_SPECID_1);
  auto obj = testObj.first;
  auto spec = testObj.second;

  // Send a specialization with a different timestamp to make sure it doesn't get picked up
  HoverObjectiveType errorSpec;
  errorSpec.specializationReferenceTimestamp(arlcore::umaa::getTimestamp());
  errorSpec.specializationReferenceID(TEST_SPECID_1);
  hoverRS_->send(errorSpec);

  hoverRS_->send(spec);

  auto sendableSpec =
      std::dynamic_pointer_cast<DefaultSendableSpec<HoverObjectiveType>>(factory_.createSpecialization(obj));
  EXPECT_EQ(sendableSpec->getSpecializationID(), obj.specializationID());
  EXPECT_EQ(sendableSpec->getSpecializationTimestamp(), obj.specializationTimestamp());
  EXPECT_EQ(sendableSpec->getSpecializationTopic(), HoverObjectiveTypeTopic);
  EXPECT_EQ(sendableSpec->getSpecializationData(), spec);
}

TEST_F(ObjectiveSpecFactory_test, MakeRoute) {
  auto testObj = makeObjective<RouteObjectiveType>(RouteObjectiveTypeTopic, TEST_SPECID_2);
  auto obj = testObj.first;
  auto spec = testObj.second;
  for (const auto& wpt : waypoints_) {
    wptLLRS_->append(wpt);
  }
  spec.waypointsListMetadata(wptLLRS_->getMetadata());

  EXPECT_CALL(*routeRS_, readInstance).WillOnce(DoAll(SetArgPointee<1>(spec), Return(ReadStatus::SUCCESS)));

  auto sendableSpec = std::dynamic_pointer_cast<RouteSendableSpec>(factory_.createSpecialization(obj));

  EXPECT_EQ(sendableSpec->getWaypoints().size(), WAYPOINT_COUNT);
  EXPECT_EQ(sendableSpec->getSpecializationData(), spec);
}

/* --------------------- */
/*      Error Tests      */
/* --------------------- */

TEST_F(ObjectiveSpecFactory_test, MakeSpecDisposed) {
  auto testObj = makeObjective<HoverObjectiveType>(HoverObjectiveTypeTopic, TEST_SPECID_1);
  auto obj = testObj.first;
  auto spec = testObj.second;

  hoverRS_->dispose(spec);

  EXPECT_FALSE(factory_.createSpecialization(obj)) << "Expected nullptr.";
}

TEST_F(ObjectiveSpecFactory_test, MakeSpecNoData) {
  auto testObj = makeObjective<HoverObjectiveType>(HoverObjectiveTypeTopic, TEST_SPECID_1);
  auto obj = testObj.first;

  EXPECT_FALSE(factory_.createSpecialization(obj)) << "Expected nullptr.";
}

TEST_F(ObjectiveSpecFactory_test, MakeSpecMissingReader) {
  auto testObj = makeObjective<DriftObjectiveType>(DriftObjectiveTypeTopic, TEST_SPECID_1);
  auto obj = testObj.first;
  auto spec = testObj.second;

  EXPECT_FALSE(factory_.createSpecialization(obj)) << "Expected nullptr.";
}

TEST_F(ObjectiveSpecFactory_test, InvalidSpecTopic) {
  ObjectiveType obj;
  obj.specializationTopic("INVALID");

  EXPECT_FALSE(factory_.createSpecialization(obj)) << "Expected nullptr.";
}
