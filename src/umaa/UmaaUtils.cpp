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

#include "UmaaUtils.h"
#include "RealtimeSystemClock.h"

namespace UMAA::Common::Measurement {

bool operator>(const DateTime &lhs, const DateTime &rhs) {
  if (lhs.seconds() > rhs.seconds()) {
    return true;
  } else if (lhs.seconds() == rhs.seconds()) {
    return lhs.nanoseconds() > rhs.nanoseconds();
  } else {
    return false;
  }
}

bool operator>=(const DateTime &lhs, const DateTime &rhs) { return lhs > rhs || lhs == rhs; }

bool operator<(const DateTime &lhs, const DateTime &rhs) {
  if (lhs.seconds() < rhs.seconds()) {
    return true;
  } else if (lhs.seconds() == rhs.seconds()) {
    return lhs.nanoseconds() < rhs.nanoseconds();
  } else {
    return false;
  }
}

bool operator<=(const DateTime &lhs, const DateTime &rhs) { return lhs < rhs || lhs == rhs; }

}  // namespace UMAA::Common::Measurement

namespace UMAA::MM::Conditional {

bool operator<(const ConditionalType &lhs, const ConditionalType &rhs) {
  if (lhs.conditionalID() < rhs.conditionalID()) {
    return true;
  } else {
    return false;
  }
}

}  // namespace UMAA::MM::Conditional

namespace arlcore::umaa {

DateTime getTimestamp() {
  timestamp t = SYSTEM_CLOCK.getCurrentTime_timestamp();
  DateTime timestamp(t.seconds, t.nanoseconds);
  return timestamp;
}

size_t hashNumericGuid(const NumericGUID& guid) {
  size_t hash = 0;
  for (int i = 0; i < 16; i++) {
    hash ^= static_cast<size_t>(guid[i]) << (8 * i % sizeof(size_t));
  }

  return hash;
}

}  // namespace arlcore::umaa
