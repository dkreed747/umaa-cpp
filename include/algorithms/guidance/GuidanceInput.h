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

#ifndef INCLUDE_ALGORITHMS_GUIDANCE_GUIDANCEINPUT_H_
#define INCLUDE_ALGORITHMS_GUIDANCE_GUIDANCEINPUT_H_

#include <cmath>

#include "InternalTypes.h"

namespace arl::algorithm {

constexpr auto kInputTolerance = 0.01;

struct GuidanceInput {
  //! \brief X vehicle position in ECEF
  flt64_t xPos;
  //! \brief Y vehicle position in ECEF
  flt64_t yPos;
  //! \brief Z vehicle position in ECEF
  flt64_t zPos;

  //! \brief Target waypoint latitude in degrees
  flt64_t latTargetDeg;
  //! \brief Target waypoint longitude in degrees
  flt64_t lonTargetDeg;
  //! \brief Target yaw to achieve waypoint in radians
  flt64_t yawTargetRad;
  //! \brief Target radius of curvature in meters
  flt64_t radiusOfCurvatureM;

  //! \brief If the algorithm should guide to line or guide to point
  bool guideToLine;

  //! \brief The distance to project a target point ahead of the vehicle. Default 50. Used for GuideToLine
  int32_t leadDistanceMeters;

  //! \brief Current latitude from global pose
  flt64_t latCurrentDeg;
  //! \brief Current longitude from global pose
  flt64_t lonCurrentDeg;

  //! \brief Prev target latitude
  flt64_t latPrevTargetDeg;
  //! \brief Prev target longitude
  flt64_t lonPrevTargetDeg;

  bool operator==(const GuidanceInput &other) const {
    return (std::abs(xPos - other.xPos) <
            kInputTolerance) &&
           (std::abs(yPos - other.yPos) <
            kInputTolerance) &&
           (std::abs(zPos - other.zPos) <
            kInputTolerance) &&
           (std::abs(latTargetDeg - other.latTargetDeg) <
            kInputTolerance) &&
           (std::abs(lonTargetDeg - other.lonTargetDeg) <
            kInputTolerance) &&
           (std::abs(yawTargetRad - other.yawTargetRad) <
            kInputTolerance) &&
           (std::abs(radiusOfCurvatureM - other.radiusOfCurvatureM) <
            kInputTolerance);
  }
};
}  // namespace arl::algorithm
#endif  // INCLUDE_ALGORITHMS_GUIDANCE_GUIDANCEINPUT_H_
