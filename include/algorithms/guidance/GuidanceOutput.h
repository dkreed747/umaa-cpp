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

#ifndef INCLUDE_ALGORITHMS_GUIDANCE_GUIDANCEOUTPUT_H_
#define INCLUDE_ALGORITHMS_GUIDANCE_GUIDANCEOUTPUT_H_

#include "InternalTypes.h"

namespace arl::algorithm {

constexpr auto kHeadingTolerance = 0.001;
constexpr auto kSpeedTolerance = 0.01;
constexpr auto kDepthTolerance = 0.01;

struct GuidanceOutput {
  //! \brief vehicle heading in radians
  flt64_t headingRad;

  //! \brief vehicle speed in meters per second
  flt64_t speedMps;

  //! \brief vehicle depth in meters below sea level (BSL)
  flt64_t depth;
};

}  // namespace arl::algorithm
#endif  // INCLUDE_ALGORITHMS_GUIDANCE_GUIDANCEOUTPUT_H_
