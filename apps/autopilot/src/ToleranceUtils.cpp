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

#include "ToleranceUtils.h"

#include <cmath>

#include "Logger.h"

namespace arlcore::autopilot::tolerance {

using UMAA::Common::Orientation::DirectionRequirementVariantTypeEnum;
using UMAA::Common::Speed::SpeedRequirementVariantTypeEnum;
using UMAA::Common::Speed::VariableSpeedVariantTypeEnum;
using UMAA::Common::Measurement::ElevationRequirementVariantTypeEnum;

namespace {
//! \brief Half-width of a [lower, upper] tolerance band.
double bandHalfWidth(double lower, double upper) {
  return std::abs(upper - lower) / 2.0;
}
}  // namespace

std::optional<DirectionValue> extractDirection(
    const UMAA::Common::Orientation::DirectionRequirementVariantType& dir) {
  const auto& sub = dir.DirectionRequirementVariantTypeSubtypes();
  DirectionValue out;
  switch (sub._d()) {
    case DirectionRequirementVariantTypeEnum::DIRECTIONTRUENORTHREQUIREMENTVARIANT_D: {
      const auto& req = sub.DirectionTrueNorthRequirementVariantVariant().direction();
      out.headingRad = req.direction();
      if (req.directionTolerance().has_value()) {
        out.toleranceRad = bandHalfWidth(req.directionTolerance().value().lowerlimit(),
                                         req.directionTolerance().value().upperlimit());
      }
      return out;
    }
    case DirectionRequirementVariantTypeEnum::DIRECTIONMAGNETICNORTHREQUIREMENTVARIANT_D: {
      // Passed through as a heading; magnetic-vs-true offset is the platform's concern.
      const auto& req = sub.DirectionMagneticNorthRequirementVariantVariant().direction();
      out.headingRad = req.direction();
      if (req.directionTolerance().has_value()) {
        out.toleranceRad = bandHalfWidth(req.directionTolerance().value().lowerlimit(),
                                         req.directionTolerance().value().upperlimit());
      }
      return out;
    }
    default:
      UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Unsupported direction variant for autopilot heading control")
      return std::nullopt;
  }
}

std::optional<SpeedValue> extractSpeed(
    const UMAA::Common::Speed::SpeedRequirementVariantType& speed) {
  const auto& sub = speed.SpeedRequirementVariantTypeSubtypes();
  SpeedValue out;
  switch (sub._d()) {
    case SpeedRequirementVariantTypeEnum::GROUNDSPEEDREQUIREMENTVARIANT_D: {
      const auto& req = sub.GroundSpeedRequirementVariantVariant().speed();
      out.speedMps = req.speed();
      if (req.speedTolerance().has_value()) {
        out.toleranceMps = bandHalfWidth(req.speedTolerance().value().lowerlimit(),
                                         req.speedTolerance().value().upperlimit());
      }
      return out;
    }
    case SpeedRequirementVariantTypeEnum::WATERSPEEDREQUIREMENTVARIANT_D: {
      const auto& req = sub.WaterSpeedRequirementVariantVariant().speed();
      out.speedMps = req.speed();
      if (req.speedTolerance().has_value()) {
        out.toleranceMps = bandHalfWidth(req.speedTolerance().value().lowerlimit(),
                                         req.speedTolerance().value().upperlimit());
      }
      return out;
    }
    default:
      UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Unsupported speed variant for autopilot speed control")
      return std::nullopt;
  }
}

std::optional<SpeedValue> extractSpeed(
    const UMAA::Common::Speed::VariableSpeedVariantType& speed) {
  const auto& sub = speed.VariableSpeedVariantTypeSubtypes();
  switch (sub._d()) {
    case VariableSpeedVariantTypeEnum::REQUIREDSPEEDVARIANT_D:
      // RequiredSpeedVariantType wraps a SpeedRequirementVariantType.
      return extractSpeed(sub.RequiredSpeedVariantVariant().speed());
    default:
      // RECOMMENDED (SpeedVariantType) and TIMEWITHSPEED variants are not supported for
      // waypoint speed control.
      UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Unsupported variable speed variant for waypoint speed")
      return std::nullopt;
  }
}

std::optional<ElevationValue> extractElevation(
    const UMAA::Common::Measurement::ElevationRequirementVariantType& elevation) {
  const auto& sub = elevation.ElevationRequirementVariantTypeSubtypes();
  ElevationValue out;
  switch (sub._d()) {
    case ElevationRequirementVariantTypeEnum::DEPTHREQUIREMENTVARIANT_D: {
      const auto& req = sub.DepthRequirementVariantVariant().depth();
      out.valueM = req.depth();
      out.frame = ElevationFrame::DEPTH;
      if (req.depthTolerance().has_value()) {
        out.toleranceM = bandHalfWidth(req.depthTolerance().value().lowerLimit(),
                                       req.depthTolerance().value().upperlimit());
      }
      return out;
    }
    case ElevationRequirementVariantTypeEnum::ALTITUDEMSLREQUIREMENTVARIANT_D:
      out.valueM = sub.AltitudeMSLRequirementVariantVariant().altitude().altitude();
      out.frame = ElevationFrame::ALTITUDE_MSL;
      return out;
    case ElevationRequirementVariantTypeEnum::ALTITUDEAGLREQUIREMENTVARIANT_D:
      out.valueM = sub.AltitudeAGLRequirementVariantVariant().altitude().altitude();
      out.frame = ElevationFrame::ALTITUDE_AGL;
      return out;
    case ElevationRequirementVariantTypeEnum::ALTITUDEGEODETICREQUIREMENTVARIANT_D:
      out.valueM = sub.AltitudeGeodeticRequirementVariantVariant().altitude().altitude();
      out.frame = ElevationFrame::ALTITUDE_GEODETIC;
      return out;
    default:
      UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Unsupported elevation variant for autopilot elevation control")
      return std::nullopt;
  }
}

AttitudeValue extractYaw(const UMAA::Common::Orientation::Orientation3DNEDRequirement& attitude) {
  AttitudeValue out;
  out.yawRad = attitude.yawZ().yaw().yaw();
  if (attitude.yawZ().yawTolerance().has_value()) {
    out.yawToleranceRad = bandHalfWidth(attitude.yawZ().yawTolerance().value().lowerlimit().yaw(),
                                        attitude.yawZ().yawTolerance().value().upperlimit().yaw());
  }
  return out;
}

std::optional<double> extractTrackToleranceM(
    const UMAA::Common::Distance::DistanceRequirementType& trackTolerance) {
  // The track tolerance's distance field is the allowed cross-track distance from the line.
  return trackTolerance.distance();
}

}  // namespace arlcore::autopilot::tolerance
