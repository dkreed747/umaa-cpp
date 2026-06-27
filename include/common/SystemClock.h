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

#ifndef INCLUDE_COMMON_SYSTEMCLOCK_H_
#define INCLUDE_COMMON_SYSTEMCLOCK_H_

#include "InternalTypes.h"

namespace arlcore {

constexpr uint64_t MICROSECONDS_PER_SEC = 1000000;
constexpr uint64_t NANOSECONDS_PER_SEC = MICROSECONDS_PER_SEC * 1000;

struct timestamp {
  int64_t seconds;
  int32_t nanoseconds;
};

//! \brief Comparison operator overload for == (equal to)
//! \param lhs The left hand side of the comparison
//! \param rhs The right hand side of the comparison
//! \return true if the left hand side is equal to the right hand side, false otherwise
bool operator==(const timestamp &lhs, const timestamp &rhs);

//! \brief Comparison operator overload for > (greater than)
//! \param lhs The left hand side of the comparison
//! \param rhs The right hand side of the comparison
//! \return true if the left hand side is greater than the right hand side, false otherwise
bool operator>(const timestamp &lhs, const timestamp &rhs);

//! \brief Comparison operator overload for >= (greater than or equal to)
//! \param lhs The left hand side of the comparison
//! \param rhs The right hand side of the comparison
//! \return true if the left hand side is greater than or equal to the right hand side, false otherwise
bool operator>=(const timestamp &lhs, const timestamp &rhs);

//! \brief Comparison operator overload for < (less than)
//! \param lhs The left hand side of the comparison
//! \param rhs The right hand side of the comparison
//! \return true if the left hand side is less than the right hand side, false otherwise
bool operator<(const timestamp &lhs, const timestamp &rhs);

//! \brief Comparison operator overload for <= (less than or equal to)
//! \param lhs The left hand side of the comparison
//! \param rhs The right hand side of the comparison
//! \return true if the left hand side is less than or equal to the right hand side, false otherwise
bool operator<=(const timestamp &lhs, const timestamp &rhs);

//! \brief Abstract class representing a consistent interface to the system clock across components.
class SystemClock {
 public:
  //! \brief Constructor
  SystemClock() = default;

  //! \brief Destructor
  virtual ~SystemClock() = default;

  //! \brief Get the current system time in nanoseconds since epoch
  //! \return Time since epoch in nanoseconds.
  virtual uint64_t getCurrentTime_nanoseconds() const = 0;

  //! \brief Get the current system time in microseconds since epoch
  //! \return Time since epoch in microseconds.
  virtual uint64_t getCurrentTime_microseconds() const = 0;

  //! \brief Get the current system time in milliseconds since epoch
  //! \return Time since epoch in milliseconds.
  virtual uint64_t getCurrentTime_milliseconds() const = 0;

  //! \brief Get the current system time in seconds since epoch
  //! \return Time since epoch in seconds.
  virtual uint64_t getCurrentTime_seconds() const = 0;

  //! \brief Get the current system time in seconds since epoch with with fractional seconds
  //! \return Time since epoch in seconds as a floating point number
  virtual flt64_t getCurrentTime_seconds_float() const = 0;

  //! \brief Get the current system time in seconds with the remainder of nanoseconds
  //! \deprecated Use getCurrentTime_timestamp instead
  //! \param[out] seconds Time since epoch in seconds
  //! \param[out] nanos   The number of nanoseconds elapsed within the current second
  //! \pre The seconds and nanos must not be nullptrs
  virtual void getCurrentTime_umaa(uint64_t *seconds, uint64_t *nanos) const = 0;

  //! \brief Returns a struct containing the current system time in seconds with the remainder of nanoseconds
  //! \return A timestamp struct with the current time
  virtual timestamp getCurrentTime_timestamp() const = 0;
};
}  // namespace arlcore
#endif  // INCLUDE_COMMON_SYSTEMCLOCK_H_
