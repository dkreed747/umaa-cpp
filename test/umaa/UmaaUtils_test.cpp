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

#include "UmaaUtils.h"
#include "RealtimeSystemClock.h"

TEST(UmaaUtilsTest, TestComparisonOperators) {
  DateTime t1(1, 0);
  DateTime t2(0, 100);
  DateTime t3(1, 0);
  EXPECT_EQ(t1, t3);
  EXPECT_GT(t1, t2);
  EXPECT_GE(t1, t3);
  EXPECT_LT(t2, t1);
  EXPECT_LE(t2, t1);
}

TEST(UmaaUtilsTest, TestHashFunction) {
  const NumericGUID id1 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0};
  const NumericGUID id2 = {0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  EXPECT_NE(arlcore::umaa::hashNumericGuid(id1), arlcore::umaa::hashNumericGuid(id2));
}

TEST(UmaaUtilsTest, TestTimestamp) {
  arlcore::RealtimeSystemClock clk;
  DateTime timestamp = arlcore::umaa::getTimestamp();
  auto sysTime = clk.getCurrentTime_timestamp();
  // Only check seconds for edge case where second advances between calls
  EXPECT_NEAR(timestamp.seconds(), sysTime.seconds, 1);
}