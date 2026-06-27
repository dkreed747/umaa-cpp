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

#include "GlobalWaypointControlServiceConsumer.h"
#include "LocalReaderSender.h"

using arlcore::io::ReadStatus;
using arlcore::io::SendStatus;

class GlobalWaypointControlServiceConsumerTest : public ::testing::Test {
 protected:
  GlobalWaypointControlServiceConsumerTest() {
    UMAA::Common::Measurement::NumericGUID srcId = {
        68, 101, 118, 111, 110, 39, 115, 32, 67, 111, 110, 115, 117, 109, 101, 114};
    UMAA::Common::Measurement::NumericGUID destId = {
        70, 97, 107, 101, 86, 101, 99, 116, 111, 114, 80, 114, 111, 118, 0, 0};

    // CONSUMER
    commandHeader_.sourceId.id(srcId);
    commandHeader_.destinationId.id(destId);

    io_ = std::make_shared<arlcore::umaa::GlobalWaypointControlServiceConsumerIo>(
        cmdSender_ = std::make_shared<arlcore::io::LocalReaderSender<arlcore::umaa::GlobalWaypointCommandType>>(),
        ackReader_ =
            std::make_shared<arlcore::io::LocalReaderSender<arlcore::umaa::GlobalWaypointCommandAckReportType>>(),
        statReader_ =
            std::make_shared<arlcore::io::LocalReaderSender<arlcore::umaa::GlobalWaypointCommandStatusType>>(),
        exeStatReader_ =
            std::make_shared<arlcore::io::LocalReaderSender<arlcore::umaa::GlobalWaypointExecutionStatusReportType>>(),
        elemSender_ = std::make_shared<
            arlcore::io::LocalReaderSender<arlcore::umaa::GlobalWaypointCommandTypeWaypointsListElement>>());

    // Test samples
    for (uint16_t i = 1; i <= 5; i++) {
      arlcore::umaa::GlobalWaypointType wpt;
      wpt.position().value().geodeticLongitude(i * 10);
      wpt.position().value().geodeticLatitude((i + 1) * 10);
      testWaypoints_.push_back(wpt);
    }
  }

  arlcore::umaa::domain::CommandHeader commandHeader_;
  std::vector<arlcore::umaa::GlobalWaypointType> testWaypoints_;

  std::shared_ptr<arlcore::umaa::GlobalWaypointControlServiceConsumerIo> io_;

  std::shared_ptr<arlcore::io::LocalReaderSender<arlcore::umaa::GlobalWaypointCommandType>> cmdSender_;
  std::shared_ptr<arlcore::io::LocalReaderSender<arlcore::umaa::GlobalWaypointCommandAckReportType>> ackReader_;
  std::shared_ptr<arlcore::io::LocalReaderSender<arlcore::umaa::GlobalWaypointCommandStatusType>> statReader_;
  std::shared_ptr<arlcore::io::LocalReaderSender<arlcore::umaa::GlobalWaypointExecutionStatusReportType>>
      exeStatReader_;
  std::shared_ptr<arlcore::io::LocalReaderSender<arlcore::umaa::GlobalWaypointCommandTypeWaypointsListElement>>
      elemSender_;
};

TEST_F(GlobalWaypointControlServiceConsumerTest, ConstructorWithList) {
  arlcore::umaa::GlobalWaypointControlServiceConsumer wptConsumer(commandHeader_, io_, testWaypoints_);

  ASSERT_EQ(wptConsumer.getMetadata().size(), 5);
  ASSERT_EQ(wptConsumer.clear(), SendStatus::SUCCESS);
  elemSender_->clear();
}

TEST_F(GlobalWaypointControlServiceConsumerTest, WaypointCommandSession) {
  arlcore::umaa::GlobalWaypointControlServiceConsumer wptConsumer(commandHeader_, io_);

  arlcore::umaa::GlobalWaypointCommandType wptCmd;
  wptCmd.timeStamp().seconds(10);

  for (auto wpt : testWaypoints_) {
    EXPECT_EQ(SendStatus::SUCCESS, wptConsumer.append(wpt));
  }

  EXPECT_EQ(SendStatus::SUCCESS, wptConsumer.send(&wptCmd));

  ASSERT_EQ(cmdSender_->count(), 1);

  ASSERT_EQ(2 * testWaypoints_.size() - 1, elemSender_->count());

  arlcore::umaa::GlobalWaypointCommandType recvCmd;
  ASSERT_EQ(cmdSender_->read(&recvCmd), ReadStatus::SUCCESS);
  EXPECT_EQ(wptConsumer.getMetadata(), recvCmd.waypointsListMetadata());
  EXPECT_EQ(wptConsumer.getCmdHeader().sessionId.getGuid(), recvCmd.sessionID());
  EXPECT_EQ(wptConsumer.getCmdHeader().destinationId.id(), recvCmd.destination().id());
  EXPECT_EQ(wptConsumer.getCmdHeader().sourceId.id(), recvCmd.source().id());
  EXPECT_EQ(wptConsumer.closeCommandSession(), SendStatus::SUCCESS);

  ASSERT_EQ(cmdSender_->count(), 1);
  EXPECT_EQ(cmdSender_->read(&recvCmd), ReadStatus::DISPOSED);
}
