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

#include "SpeedStatusData.h"
#include "EnumSpecifiers.h"

TEST(SpeedStatusDataTest, setSpeedOverGround) {
  arlcore::SpeedStatusData ssData;

  flt64_t expVal = 10.0;

  ssData.setSpeedOverGround(expVal);
  EXPECT_EQ(expVal, ssData.getSpeedOverGround());
}

TEST(SpeedStatusDataTest, setSpeedThroughAir) {
  arlcore::SpeedStatusData ssData;

  flt64_t expVal = 1.0;

  ssData.setSpeedThroughAir(expVal);
  EXPECT_EQ(expVal, ssData.getSpeedThroughAir());
}

TEST(SpeedStatusDataTest, setSpeedThroughWater) {
  arlcore::SpeedStatusData ssData;

  flt64_t expVal = 9.2;

  ssData.setSpeedThroughWater(expVal);
  EXPECT_EQ(expVal, ssData.getSpeedThroughWater());
}

TEST(SpeedStatusDataTest, getSpeedFromSpecifier) {
  arlcore::SpeedStatusData ssData;

  flt64_t RPM = 49.42;
  flt64_t SOG = 42.58;
  flt64_t STA = 44.96;
  flt64_t STW = 26.10;
  flt64_t VSP = -1.0;  // VEHICLE_SPEED not implemented in GlobalPoseData.h

  ssData.setSpeedOverGround(SOG);
  ssData.setSpeedThroughAir(STA);
  ssData.setSpeedThroughWater(STW);

  EXPECT_EQ(ssData.getSpeedFromTypeSpecifier(arlcore::Speeds::SPEED_OVER_GROUND), SOG);
  EXPECT_EQ(ssData.getSpeedFromTypeSpecifier(arlcore::Speeds::SPEED_THROUGH_AIR), STA);
  EXPECT_EQ(ssData.getSpeedFromTypeSpecifier(arlcore::Speeds::SPEED_THROUGH_WATER), STW);
  EXPECT_EQ(ssData.getSpeedFromTypeSpecifier(arlcore::Speeds::VEHICLE_SPEED), VSP);

  // Force bad speed type to test the catch all
  EXPECT_EQ(ssData.getSpeedFromTypeSpecifier(static_cast<arlcore::Speeds>(6)), -1.0);
}
