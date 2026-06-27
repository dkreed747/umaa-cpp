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

#ifndef INCLUDE_COMMON_REALTIMESYSTEMCLOCK_H_
#define INCLUDE_COMMON_REALTIMESYSTEMCLOCK_H_

#include <chrono>

#include "SystemClock.h"

namespace arlcore {

//! \brief ComponentClockBase implementation that uses std::chrono and system_clock to get the current system time.
class RealtimeSystemClock : public SystemClock {
  using SystemClock::SystemClock;

 public:
  RealtimeSystemClock() = default;

  virtual ~RealtimeSystemClock() = default;

  //! \brief Get the current system time in nanoseconds since epoch
  //! \return Time since epoch in nanoseconds
  uint64_t getCurrentTime_nanoseconds() const override {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
           std::chrono::system_clock::now().time_since_epoch()).count();
  }

  //! \brief Get the current system time in microseconds since epoch
  //! \return Time since epoch in microseconds
  uint64_t getCurrentTime_microseconds() const override {
    return std::chrono::duration_cast<std::chrono::microseconds>(
           std::chrono::system_clock::now().time_since_epoch()).count();
  }

  //! \brief Get the current system time in milliseconds since epoch
  //! \return Time since epoch in milliseconds
  uint64_t getCurrentTime_milliseconds() const override {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
           std::chrono::system_clock::now().time_since_epoch()).count();
  }

  //! \brief Get the current system time in seconds since epoch
  //! \return Time since epoch in seconds
  uint64_t getCurrentTime_seconds() const override {
    return std::chrono::duration_cast<std::chrono::seconds>(
           std::chrono::system_clock::now().time_since_epoch()).count();
  }

  //! \brief Get the current system time in seconds since epoch with with fractional seconds
  //! \return Time since epoch in seconds as a floating point number
  flt64_t getCurrentTime_seconds_float() const override {
    flt64_t micros_f = static_cast<flt64_t>(std::chrono::duration_cast<std::chrono::microseconds>(
                       std::chrono::system_clock::now().time_since_epoch()).count());
    return micros_f / MICROSECONDS_PER_SEC;
  }

  //! \brief Get the current system time in seconds with the remainder of nanoseconds
  //! \deprecated Use getCurrentTime_timestamp instead
  //! \param[out] seconds Time since epoch in seconds
  //! \param[out] nanos   The number of nanoseconds elapsed within the current second
  //! \pre The seconds and nanos must not be nullptrs
  void getCurrentTime_umaa(uint64_t *seconds, uint64_t *nanos) const override {
    uint64_t nanosec = std::chrono::duration_cast<std::chrono::nanoseconds>(
             std::chrono::system_clock::now().time_since_epoch()).count();
    *seconds = nanosec / NANOSECONDS_PER_SEC;
    *nanos = nanosec % NANOSECONDS_PER_SEC;
  }

  //! \brief Returns a struct containing the current system time in seconds with the remainder of nanoseconds
  //! \return A timestamp struct with the current time
  timestamp getCurrentTime_timestamp() const override {
    timestamp result;
    int64_t nanosec = std::chrono::duration_cast<std::chrono::nanoseconds>(
             std::chrono::system_clock::now().time_since_epoch()).count();
    result.seconds = nanosec / NANOSECONDS_PER_SEC;
    result.nanoseconds = nanosec % NANOSECONDS_PER_SEC;
    return result;
  }
};

static const RealtimeSystemClock SYSTEM_CLOCK;

}  // namespace arlcore
#endif  // INCLUDE_COMMON_REALTIMESYSTEMCLOCK_H_
