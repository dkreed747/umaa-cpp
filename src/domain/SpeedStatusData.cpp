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

#include <string>

#include "SpeedStatusData.h"
#include "Logger.h"

namespace arlcore {

const flt64_t& SpeedStatusData::getSpeedOverGround() const {
  return speedOverGround_;
}

const flt64_t& SpeedStatusData::getSpeedThroughAir() const {
  return speedThroughAir_;
}

const flt64_t& SpeedStatusData::getSpeedThroughWater() const {
  return speedThroughWater_;
}

flt64_t SpeedStatusData::getSpeedFromTypeSpecifier(const Speeds& specifier) const {
  flt64_t retVal = -1.0;
  switch (specifier) {
    case SPEED_OVER_GROUND:
      retVal = speedOverGround_;
      break;
    case SPEED_THROUGH_AIR:
      retVal = speedThroughAir_;
      break;
    case SPEED_THROUGH_WATER:
      retVal = speedThroughWater_;
      break;
    default:
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER,
        "Speed type specifier (" << specifier << ") Not supported")
  }
  return retVal;
}

void SpeedStatusData::setSpeedOverGround(const flt64_t& inSpeed) {
  speedOverGround_ = inSpeed;
}

void SpeedStatusData::setSpeedThroughAir(const flt64_t& inSpeed) {
  speedThroughAir_ = inSpeed;
}

void SpeedStatusData::setSpeedThroughWater(const flt64_t& inSpeed) {
  speedThroughWater_ = inSpeed;
}

}  // namespace arlcore
