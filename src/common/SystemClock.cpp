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

#include "SystemClock.h"

namespace arlcore {

bool operator==(const timestamp& lhs, const timestamp& rhs) {
  return lhs.seconds == rhs.seconds && lhs.nanoseconds == rhs.nanoseconds;
}

bool operator>(const timestamp& lhs, const timestamp& rhs) {
  if (lhs.seconds > rhs.seconds) {
    return true;
  } else if (lhs.seconds == rhs.seconds) {
    return lhs.nanoseconds > rhs.nanoseconds;
  } else {
    return false;
  }
}

bool operator>=(const timestamp& lhs, const timestamp& rhs) {
  return lhs > rhs || lhs == rhs;
}

bool operator<(const timestamp& lhs, const timestamp& rhs) {
  if (lhs.seconds < rhs.seconds) {
    return true;
  } else if (lhs.seconds == rhs.seconds) {
    return lhs.nanoseconds < rhs.nanoseconds;
  } else {
    return false;
  }
}

bool operator<=(const timestamp& lhs, const timestamp& rhs) { return lhs < rhs || lhs == rhs; }

}  // namespace arlcore
