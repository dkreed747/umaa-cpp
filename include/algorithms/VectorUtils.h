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
#ifndef INCLUDE_ALGORITHMS_VECTORUTILS_H_
#define INCLUDE_ALGORITHMS_VECTORUTILS_H_

#include <math.h>

#include <UMAA/MO/GlobalVectorControl/GlobalVectorCommandType.hpp>

#include "InternalTypes.h"
#include "Logger.h"

namespace arl::algorithm {

using UMAA::MO::GlobalVectorControl::GlobalVectorCommandType;
using UMAA::Common::Orientation::DirectionRequirementVariantTypeEnum;
using UMAA::Common::Speed::SpeedRequirementVariantTypeEnum;
using UMAA::Common::Measurement::ElevationRequirementVariantTypeEnum;


// Default tolerances for compare vectors
const flt64_t DIRECTION_TOLERANCE = 0.1;
const flt64_t SPEED_TOLERANCE = 0.1;
const flt64_t ELEVATION_TOLERANCE = 0.1;
const flt64_t PITCH_RATE_TOLERANCE = 0.1;

//! \brief Calculate the effective difference between two angles
//! \param a The first angle (Radians)
//! \param b The second angle (Radians)
//! \return The difference between the normalized angles
static flt64_t angleDiffRadians(flt64_t a, flt64_t b) {
  a = remainder(a, 2 * M_PI);
  b = remainder(b, 2 * M_PI);
  flt64_t diff = b - a;
  if (fabs(diff) > M_PI) {
    diff = (diff > 0 ? 1 : -1) * 2 * M_PI - diff;
  }
  return diff;
}

//! \brief Determine if two vector commands are similar within a certain tolerance
//! \param a The first vector command
//! \param b The second vector command
//! \param directionTolerance The tolerance for differences in commanded direction
//! \param speedTolerance The tolerance for differences in commanded speed
//! \param elevationTolerance The tolerance for differences in commanded elevation
//! \param pitchRateTolerance The tolerance for differences in commanded pitch rate
//! \return Whether or not the provided commands are similar within the provided tolerances
static bool areVectorCommandsSimilar(
    const GlobalVectorCommandType& a,
    const GlobalVectorCommandType& b,
    const flt64_t directionTolerance = DIRECTION_TOLERANCE,
    const flt64_t speedTolerance = SPEED_TOLERANCE,
    const flt64_t elevationTolerance = ELEVATION_TOLERANCE,
    const flt64_t pitchRateTolerance = PITCH_RATE_TOLERANCE) {
  if (a.directionMode() != b.directionMode()) {
    UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Attempted to compare vector commands with different direction modes")
    return false;
  }
  if (a.direction().DirectionRequirementVariantTypeSubtypes()._d() !=
      b.direction().DirectionRequirementVariantTypeSubtypes()._d()) {
    UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Attempted to compare vector commands with different direction variant types")
    return false;
  }
  if (a.speed().SpeedRequirementVariantTypeSubtypes()._d() !=
      b.speed().SpeedRequirementVariantTypeSubtypes()._d()) {
    UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Attempted to compare vectors commands with different speed variant types")
    return false;
  }
  if (a.speed().SpeedRequirementVariantTypeSubtypes()._d() ==
      SpeedRequirementVariantTypeEnum::VEHICLESPEEDMODEREQUIREMENTVARIANT_D &&
      a.speed().SpeedRequirementVariantTypeSubtypes().VehicleSpeedModeRequirementVariantVariant().mode() !=
      b.speed().SpeedRequirementVariantTypeSubtypes().VehicleSpeedModeRequirementVariantVariant().mode()) {
    // Vehicle speed modes differ
    return false;
  }
  if (a.elevation().has_value() != b.elevation().has_value()) {
    return false;
  }
  if (a.elevation().has_value() && a.elevation().value().ElevationRequirementVariantTypeSubtypes()._d() !=
      b.elevation().value().ElevationRequirementVariantTypeSubtypes()._d()) {
    UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Attempted to compare vector commands with different elevation variant types")
    return false;
  }
  if (a.depthChangePitch().has_value() != b.depthChangePitch().has_value()) {
    return false;
  }
  if (a.depthChangePitch().has_value() &&
      fabs(a.depthChangePitch().value().pitch().pitch() -
      b.depthChangePitch().value().pitch().pitch()) > pitchRateTolerance) {
    return false;
  }

  flt64_t directionDiff = 0;
  switch (a.direction().DirectionRequirementVariantTypeSubtypes()._d()) {
    case DirectionRequirementVariantTypeEnum::DIRECTIONCURRENTREQUIREMENTVARIANT_D:
      directionDiff = fabs(angleDiffRadians(a.direction().DirectionRequirementVariantTypeSubtypes()
        .DirectionCurrentRequirementVariantVariant().direction().direction(),
        b.direction().DirectionRequirementVariantTypeSubtypes()
        .DirectionCurrentRequirementVariantVariant().direction().direction()));
      break;
    case DirectionRequirementVariantTypeEnum::DIRECTIONMAGNETICNORTHREQUIREMENTVARIANT_D:
      directionDiff = fabs(angleDiffRadians(a.direction().DirectionRequirementVariantTypeSubtypes()
        .DirectionMagneticNorthRequirementVariantVariant().direction().direction(),
        b.direction().DirectionRequirementVariantTypeSubtypes()
        .DirectionMagneticNorthRequirementVariantVariant().direction().direction()));
      break;
    case DirectionRequirementVariantTypeEnum::DIRECTIONTRUENORTHREQUIREMENTVARIANT_D:
      directionDiff = fabs(angleDiffRadians(a.direction().DirectionRequirementVariantTypeSubtypes()
        .DirectionTrueNorthRequirementVariantVariant().direction().direction(),
        b.direction().DirectionRequirementVariantTypeSubtypes()
        .DirectionTrueNorthRequirementVariantVariant().direction().direction()));
      break;
    case DirectionRequirementVariantTypeEnum::DIRECTIONTURNRATEREQUIREMENTVARIANT_D:
      directionDiff = fabs(a.direction().DirectionRequirementVariantTypeSubtypes()
        .DirectionTurnRateRequirementVariantVariant().directionRate().directionRate() -
        b.direction().DirectionRequirementVariantTypeSubtypes().DirectionTurnRateRequirementVariantVariant()
        .directionRate().directionRate());
      break;
    case DirectionRequirementVariantTypeEnum::DIRECTIONWINDREQUIREMENTVARIANT_D:
      directionDiff = fabs(angleDiffRadians(a.direction().DirectionRequirementVariantTypeSubtypes()
        .DirectionWindRequirementVariantVariant().direction().direction(),
        b.direction().DirectionRequirementVariantTypeSubtypes().DirectionWindRequirementVariantVariant()
        .direction().direction()));
      break;
    default:
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Unexpected direction variant type.")
      return false;
  }
  if (directionDiff > directionTolerance) {
    return false;
  }

  flt64_t speedDiff = 0;
  switch (a.speed().SpeedRequirementVariantTypeSubtypes()._d()) {
    case SpeedRequirementVariantTypeEnum::AIRSPEEDREQUIREMENTVARIANT_D:
      speedDiff =
        fabs(a.speed().SpeedRequirementVariantTypeSubtypes().AirSpeedRequirementVariantVariant().speed().speed() -
        b.speed().SpeedRequirementVariantTypeSubtypes().AirSpeedRequirementVariantVariant().speed().speed());
      break;
    case SpeedRequirementVariantTypeEnum::ENGINERPMSPEEDREQUIREMENTVARIANT_D:
      speedDiff =
        fabs(a.speed().SpeedRequirementVariantTypeSubtypes().EngineRPMSpeedRequirementVariantVariant().rpm().speed() -
        b.speed().SpeedRequirementVariantTypeSubtypes().EngineRPMSpeedRequirementVariantVariant().rpm().speed());
      break;
    case SpeedRequirementVariantTypeEnum::GROUNDSPEEDREQUIREMENTVARIANT_D:
      speedDiff =
        fabs(a.speed().SpeedRequirementVariantTypeSubtypes().GroundSpeedRequirementVariantVariant().speed().speed() -
        b.speed().SpeedRequirementVariantTypeSubtypes().GroundSpeedRequirementVariantVariant().speed().speed());
      break;
    case SpeedRequirementVariantTypeEnum::VEHICLESPEEDMODEREQUIREMENTVARIANT_D:
      break;
    case SpeedRequirementVariantTypeEnum::WATERSPEEDREQUIREMENTVARIANT_D:
      speedDiff =
        fabs(a.speed().SpeedRequirementVariantTypeSubtypes().WaterSpeedRequirementVariantVariant().speed().speed() -
        b.speed().SpeedRequirementVariantTypeSubtypes().WaterSpeedRequirementVariantVariant().speed().speed());
      break;
    default:
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Unexpected speed variant type.")
      return false;
  }
  if (speedDiff > speedTolerance) {
    return false;
  }

  flt64_t elevationDiff = 0;
  switch (a.elevation().value().ElevationRequirementVariantTypeSubtypes()._d()) {
    case ElevationRequirementVariantTypeEnum::ALTITUDEAGLREQUIREMENTVARIANT_D:
      elevationDiff =
        fabs(a.elevation().value().ElevationRequirementVariantTypeSubtypes().AltitudeAGLRequirementVariantVariant()
        .altitude().altitude() - b.elevation().value().ElevationRequirementVariantTypeSubtypes()
        .AltitudeAGLRequirementVariantVariant().altitude().altitude());
      break;
    case ElevationRequirementVariantTypeEnum::ALTITUDEASFREQUIREMENTVARIANT_D:
      elevationDiff =
        fabs(a.elevation().value().ElevationRequirementVariantTypeSubtypes().AltitudeASFRequirementVariantVariant()
        .altitude().altitude() - b.elevation().value().ElevationRequirementVariantTypeSubtypes()
        .AltitudeASFRequirementVariantVariant().altitude().altitude());
      break;
    case ElevationRequirementVariantTypeEnum::ALTITUDEGEODETICREQUIREMENTVARIANT_D:
      elevationDiff =
        fabs(a.elevation().value().ElevationRequirementVariantTypeSubtypes().AltitudeGeodeticRequirementVariantVariant()
        .altitude().altitude() - b.elevation().value().ElevationRequirementVariantTypeSubtypes()
        .AltitudeGeodeticRequirementVariantVariant().altitude().altitude());
      break;
    case ElevationRequirementVariantTypeEnum::ALTITUDEMSLREQUIREMENTVARIANT_D:
      elevationDiff =
        fabs(a.elevation().value().ElevationRequirementVariantTypeSubtypes().AltitudeMSLRequirementVariantVariant()
        .altitude().altitude() - b.elevation().value().ElevationRequirementVariantTypeSubtypes()
        .AltitudeMSLRequirementVariantVariant().altitude().altitude());
      break;
    case ElevationRequirementVariantTypeEnum::ALTITUDERATEASFREQUIREMENTVARIANT_D:
      elevationDiff =
        fabs(a.elevation().value().ElevationRequirementVariantTypeSubtypes().AltitudeRateASFRequirementVariantVariant()
        .altitudeRate().altitudeRate() - b.elevation().value().ElevationRequirementVariantTypeSubtypes()
        .AltitudeRateASFRequirementVariantVariant().altitudeRate().altitudeRate());
      break;
    case ElevationRequirementVariantTypeEnum::DEPTHRATEREQUIREMENTVARIANT_D:
      elevationDiff =
        fabs(a.elevation().value().ElevationRequirementVariantTypeSubtypes().DepthRateRequirementVariantVariant()
        .depthRate().depthRate() - b.elevation().value().ElevationRequirementVariantTypeSubtypes()
        .DepthRateRequirementVariantVariant().depthRate().depthRate());
      break;
    case ElevationRequirementVariantTypeEnum::DEPTHREQUIREMENTVARIANT_D:
      elevationDiff =
        fabs(a.elevation().value().ElevationRequirementVariantTypeSubtypes().DepthRequirementVariantVariant()
        .depth().depth() - b.elevation().value().ElevationRequirementVariantTypeSubtypes()
        .DepthRequirementVariantVariant().depth().depth());
      break;
    default:
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Unexpected elevation variant type.")
      return false;
  }
  if (elevationDiff > elevationTolerance) {
    return false;
  }

  return true;
}


}  // namespace arl::algorithm
#endif  // INCLUDE_ALGORITHMS_VECTORUTILS_H_
