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

#ifndef INCLUDE_COMMON_ENUMSPECIFIERS_H_
#define INCLUDE_COMMON_ENUMSPECIFIERS_H_

#include <cstdint>
#include <iostream>

namespace arlcore {
// Corresponds to UMAA::Common::Measurement::ElevationRequirementVariantTypeEnum
enum Elevations : uint8_t {
    ABOVE_GROUND_LEVEL = 0,
    ABOVE_SEA_FLOOR = 1,
    GEODETIC = 2,
    MEAN_SEA_LEVEL = 3,
    RATE_ABOVE_SEA_FLOOR = 4,
    DEPTH_RATE = 5,
    DEPTH_BELOW_SEA_LEVEL = 6
};

// Corresponds to UMAA::Common::Orientation::DirectionRequirementVariantTypeEnum
enum Directions : uint8_t {
    CURRENT = 0,
    MAGNETIC_NORTH = 1,
    TRUE_NORTH = 2,
    TURN_RATE = 3,
    WIND = 4
};

// Corresponds to UMAA::Common::Speed::SpeedRequirementVariantTypeEnum
enum Speeds : uint8_t {
    SPEED_THROUGH_AIR = 0,
    ENGINE_RPM = 1,
    SPEED_OVER_GROUND = 2,
    VEHICLE_SPEED = 3,
    SPEED_THROUGH_WATER = 4
};

}  // namespace arlcore
#endif  // INCLUDE_COMMON_ENUMSPECIFIERS_H_
