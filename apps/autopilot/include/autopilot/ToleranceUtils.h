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

#ifndef APPS_AUTOPILOT_INCLUDE_AUTOPILOT_TOLERANCEUTILS_H_
#define APPS_AUTOPILOT_INCLUDE_AUTOPILOT_TOLERANCEUTILS_H_

#include <optional>

#include <UMAA/Common/Distance/DistanceRequirementType.hpp>
#include <UMAA/Common/Measurement/ElevationRequirementVariantType.hpp>
#include <UMAA/Common/Orientation/DirectionRequirementVariantType.hpp>
#include <UMAA/Common/Orientation/Orientation3DNEDRequirement.hpp>
#include <UMAA/Common/Speed/SpeedRequirementVariantType.hpp>
#include <UMAA/Common/Speed/VariableSpeedVariantType.hpp>

#include "ControlVector.h"

namespace arlcore::autopilot {

//! \brief A heading requirement: a value (radians, true north) with an optional tolerance
//! half-width (radians).
struct DirectionValue {
  double headingRad = 0.0;
  std::optional<double> toleranceRad;
};

//! \brief A speed requirement: a value (m/s) with an optional tolerance (m/s).
struct SpeedValue {
  double speedMps = 0.0;
  std::optional<double> toleranceMps;
};

//! \brief An elevation/depth requirement: value, frame, and optional tolerance.
struct ElevationValue {
  double valueM = 0.0;
  ElevationFrame frame = ElevationFrame::DEPTH;
  std::optional<double> toleranceM;
};

//! \brief A yaw requirement: value (radians, NED) and optional tolerance half-width (radians).
struct AttitudeValue {
  double yawRad = 0.0;
  std::optional<double> yawToleranceRad;
};

//! \brief Helpers to pull plain scalar values + tolerances out of UMAA requirement-variant
//! unions. Centralizing this keeps the union-discriminator handling in one place.
//!
//! NOTE: these traverse generated CycloneDDS-CXX union accessors; the exact accessor chains
//! are the highest-risk area to confirm at first compile in the SDK build container.
namespace tolerance {

//! \brief Extract the commanded heading + tolerance from a direction requirement. Supports
//! true-north / magnetic-north reference frames; returns nullopt for unsupported variants.
std::optional<DirectionValue> extractDirection(
    const UMAA::Common::Orientation::DirectionRequirementVariantType& dir);

//! \brief Extract ground/water speed + tolerance from a speed requirement.
std::optional<SpeedValue> extractSpeed(
    const UMAA::Common::Speed::SpeedRequirementVariantType& speed);

//! \brief Extract speed from a waypoint's variable-speed requirement (required/recommended).
std::optional<SpeedValue> extractSpeed(
    const UMAA::Common::Speed::VariableSpeedVariantType& speed);

//! \brief Extract elevation/depth value + frame + tolerance from an elevation requirement.
std::optional<ElevationValue> extractElevation(
    const UMAA::Common::Measurement::ElevationRequirementVariantType& elevation);

//! \brief Extract the arrival yaw + tolerance from a 3D NED orientation requirement.
AttitudeValue extractYaw(const UMAA::Common::Orientation::Orientation3DNEDRequirement& attitude);

//! \brief Extract the cross-track distance tolerance (meters) from a track tolerance, if set.
std::optional<double> extractTrackToleranceM(
    const UMAA::Common::Distance::DistanceRequirementType& trackTolerance);

}  // namespace tolerance
}  // namespace arlcore::autopilot
#endif  // APPS_AUTOPILOT_INCLUDE_AUTOPILOT_TOLERANCEUTILS_H_
