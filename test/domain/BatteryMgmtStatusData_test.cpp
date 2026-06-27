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

#include "BatteryMgmtStatusData.h"
#include "BatteryStatusData.h"

TEST(BatteryMgmtStatusDataTest, batteryMgmtTest) {
  arlcore::BatteryMgmtStatusData batteryMgmt;

  flt64_t expectedCurrent = 24.0;
  flt64_t expectedVoltage = 53.0;
  flt64_t expectedTemp = 1.0;

  arlcore::BatteryStatusData tempBattery;

  // Battery 1
  tempBattery.setCurrent(expectedCurrent);
  tempBattery.setVoltage(expectedVoltage);
  tempBattery.setTemp(expectedTemp);

  batteryMgmt.addBattery(tempBattery);

  // Battery 2
  tempBattery.setCurrent(expectedCurrent + 1);
  tempBattery.setVoltage(expectedVoltage + 1);
  tempBattery.setTemp(expectedTemp + 1);

  batteryMgmt.addBattery(tempBattery);

  for (int32_t i = 0; i < batteryMgmt.getNumberOfBatteries(); i++) {
    EXPECT_EQ(expectedCurrent + i, batteryMgmt.getBatteries().at(i).getCurrent());
    EXPECT_EQ(expectedVoltage + i, batteryMgmt.getBatteries().at(i).getVoltage());
    EXPECT_EQ(expectedTemp + i, batteryMgmt.getBatteries().at(i).getTemp());
  }
}
