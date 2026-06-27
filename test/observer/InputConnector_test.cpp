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

#include <memory>

#include <gtest/gtest.h>

#include "GlobalPoseData.h"
#include "InputConnector.h"
#include "Subject.h"
#include "TestObserver.h"

namespace arlcoretest {

//! \brief Test that executes the trigger function and verifies the Subject
//!  receives the updated data.
TEST(InputConnectorTest, trigger) {

  std::shared_ptr<arlcore::Subject<arlcore::GlobalPoseData>> subject = std::make_shared<arlcore::Subject<arlcore::GlobalPoseData>>();
	arlcore::InputConnector<arlcore::GlobalPoseData> gpInput(subject);

	arlcore::GlobalPoseData inData;
	inData.setLatitude(1.23456789);
	inData.setLongitude(2.34567891);
	inData.setAttitude(1.2345, 2.3456, 3.4567);

	EXPECT_NE(inData, gpInput.getLastData());

	gpInput.trigger(inData);

	// Compare the Data and verify they match what was input.
	EXPECT_EQ(inData, gpInput.getLastData());

}

TEST(InputConnectorTest, setData) {

  std::shared_ptr<arlcore::Subject<arlcore::GlobalPoseData>> subject = std::make_shared<arlcore::Subject<arlcore::GlobalPoseData>>();
  arlcore::InputConnector<arlcore::GlobalPoseData> gpInput(subject);

  arlcore::GlobalPoseData inData;
  inData.setLatitude(1.23456789);
  inData.setLongitude(2.34567891);
  inData.setAttitude(1.2345, 2.3456, 3.4567);

  EXPECT_NE(inData, gpInput.getLastData());

  gpInput.setData(inData);

  // Compare the Data and verify they match what was input.
  EXPECT_EQ(inData, gpInput.getLastData());

}

TEST(InputConnectorTest, observerCountZero) {
  std::shared_ptr<arlcore::Subject<arlcore::GlobalPoseData>> subject = std::make_shared<arlcore::Subject<arlcore::GlobalPoseData>>();
  arlcore::InputConnector<arlcore::GlobalPoseData> gpInput(subject);

  EXPECT_EQ(0, gpInput.getSubject()->getObserverCount());
}

}  // namespace arlcoretest
