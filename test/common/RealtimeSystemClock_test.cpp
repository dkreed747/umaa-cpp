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

#include "RealtimeSystemClock.h"

#include <limits>
#include <gtest/gtest.h>


TEST(RealtimeSystemClock_test, constructor) {
  arlcore::RealtimeSystemClock clock;
  uint64_t seconds;
  uint64_t nanos;  // Not checked since it may be 0 in rare cases

  // Verify system time is not 0
  EXPECT_NE(0U, clock.getCurrentTime_nanoseconds());
  EXPECT_NE(0U, clock.getCurrentTime_microseconds());
  EXPECT_NE(0U, clock.getCurrentTime_milliseconds());
  EXPECT_NE(0U, clock.getCurrentTime_seconds());
  EXPECT_NE(0.0, clock.getCurrentTime_seconds_float());
  clock.getCurrentTime_umaa(&seconds, &nanos);
  EXPECT_NE(0U, seconds);
}

TEST(RealtimeSystemClock_test, getCurrentTime_nanoseconds) {
  arlcore::RealtimeSystemClock clock;

  // Get the current time point.
  std::chrono::system_clock::time_point::duration point = std::chrono::system_clock::now().time_since_epoch();
  uint64_t actualTimeNanos =  clock.getCurrentTime_nanoseconds();

  // Convert time point to microseconds.
  uint64_t sysTimeNanos = std::chrono::duration_cast<std::chrono::nanoseconds>(point).count();

  // Allow at a maximum a 100 microsecond difference.
  uint64_t timeDiff = actualTimeNanos - sysTimeNanos;
  EXPECT_TRUE(timeDiff <= 100000);
}

TEST(RealtimeSystemClock_test, getCurrentTime_microseconds) {
  arlcore::RealtimeSystemClock clock;

  // Get the current time point.
  std::chrono::system_clock::time_point::duration point = std::chrono::system_clock::now().time_since_epoch();
  uint64_t actualTimeMicros =  clock.getCurrentTime_microseconds();

  // Convert time point to microseconds.
  uint64_t sysTimeMicros = std::chrono::duration_cast<std::chrono::microseconds>(point).count();

  // Allow at a maximum a 100 microsecond difference.
  uint64_t timeDiff = actualTimeMicros - sysTimeMicros;
  EXPECT_TRUE(timeDiff <= 100);
}

TEST(RealtimeSystemClock_test, getCurrentTime_milliseconds) {
  arlcore::RealtimeSystemClock clock;

  // Get the current time point.
  std::chrono::system_clock::time_point::duration point = std::chrono::system_clock::now().time_since_epoch();
  uint64_t sysTimeMillis = std::chrono::duration_cast<std::chrono::milliseconds>(point).count();

  // These calls are fast enough that they will match the above pulled system times.
  EXPECT_EQ(sysTimeMillis, clock.getCurrentTime_milliseconds());
}

TEST(RealtimeSystemClock_test, getCurrentTime_seconds) {
  arlcore::RealtimeSystemClock clock;

  // Get the current time point.
  std::chrono::system_clock::time_point::duration point = std::chrono::system_clock::now().time_since_epoch();
  uint64_t sysTimeSeconds = std::chrono::duration_cast<std::chrono::seconds>(point).count();

  // These calls are fast enough that they will match the above pulled system times.
  EXPECT_EQ(sysTimeSeconds, clock.getCurrentTime_seconds());
}

TEST(RealtimeSystemClock_test, getCurrentTime_seconds_float) {
  arlcore::RealtimeSystemClock clock;

  // Get the current time point.
  std::chrono::system_clock::time_point::duration point = std::chrono::system_clock::now().time_since_epoch();
  flt64_t clockTimeSeconds = clock.getCurrentTime_seconds_float();

  uint64_t sysTimeMicros = std::chrono::duration_cast<std::chrono::microseconds>(point).count();

  flt64_t sysTimeSeconds = static_cast<flt64_t>(sysTimeMicros) / arlcore::MICROSECONDS_PER_SEC;

  flt64_t timeDiff = clockTimeSeconds - sysTimeSeconds;

  EXPECT_TRUE(timeDiff <= 0.0001);
}

TEST(RealtimeSystemClock_test, getCurrentTime_umaa) {
  arlcore::RealtimeSystemClock clock;
  uint64_t seconds;
  uint64_t nanos;

  // Get the current time point.
  std::chrono::system_clock::time_point::duration point = std::chrono::system_clock::now().time_since_epoch();
  clock.getCurrentTime_umaa(&seconds, &nanos);

  uint64_t sysTimeNanos = std::chrono::duration_cast<std::chrono::nanoseconds>(point).count();

  uint64_t sysTimeSeconds = sysTimeNanos / arlcore::NANOSECONDS_PER_SEC;
  uint64_t sysTimeNanoRemainder = sysTimeNanos % arlcore::NANOSECONDS_PER_SEC;

  // These calls are fast enough that they should match the above pulled system times.
  EXPECT_EQ(sysTimeSeconds, seconds);

  uint64_t timeDiff = nanos - sysTimeNanoRemainder;
  EXPECT_TRUE(timeDiff <= 100000);
}
