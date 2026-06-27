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

#ifndef INCLUDE_ALGORITHMS_GUIDANCE_GUIDANCE_H_
#define INCLUDE_ALGORITHMS_GUIDANCE_GUIDANCE_H_

#include <math.h>
#include <optional>

#include "GeographicUtils.h"
#include "GuidanceInput.h"
#include "GuidanceOutput.h"
#include "UmaaUtils.h"

#include "UMAA/MM/BaseType/WaypointType.hpp"
#include "UMAA/SA/GlobalPoseStatus/GlobalPoseReportType.hpp"
#include "UMAA/MO/GlobalVectorControl/GlobalVectorCommandType.hpp"

namespace arl::algorithm {

using UMAA::MM::BaseType::WaypointType;
using UMAA::SA::GlobalPoseStatus::GlobalPoseReportType;
using UMAA::MO::GlobalVectorControl::GlobalVectorCommandType;

//! \brief Calculates the heading to get the to next waypoint. Implements a lead distance track tolerance algorithm
//! if the navigation mode is guide to line.
//! \param in GuidanceInput struct with all required values to do this calculation
//! \return GuidanceOutput struct with the fields required to build the GlobalVectorCommand
static std::optional<GuidanceOutput> calculateGuidanceOutput(const GuidanceInput& in) {
  GuidanceOutput out {0.0, 0.0, 0.0};

  // Convert waypoint (lat/lon) to local NED with origin at vehicle
  flt64_t wptAltitude = 0.0;
  flt64_t wptNorthM, wptEastM, wptDownM;

  // Guide to point if in.guideToLine is false
  flt64_t latitude = in.latTargetDeg;
  flt64_t longitude = in.lonTargetDeg;

  if (in.guideToLine) {
    flt64_t longitudeDistanceToPoint = arlcore::getHaversineDistance(in.latCurrentDeg, in.lonCurrentDeg,
      in.latCurrentDeg, in.lonTargetDeg);
    flt64_t latitudeDistanceToPoint = arlcore::getHaversineDistance(in.latCurrentDeg, in.lonCurrentDeg,
      in.latTargetDeg, in.lonCurrentDeg);

    flt64_t haversineDistanceToPoint = arlcore::getHaversineDistance(in.latCurrentDeg, in.lonCurrentDeg,
                                                                    in.latTargetDeg, in.lonTargetDeg);

    flt64_t ratioOfHaversineToLeadPoint = haversineDistanceToPoint / in.leadDistanceMeters;

    flt64_t headingBetweenCurrentAndNextPoint = arlcore::calculateHeadingBetweenWaypoints(in.latTargetDeg,
      in.lonTargetDeg, in.latCurrentDeg, in.lonCurrentDeg);

    UMAA_LOG_DEBUG(util::SYSTEM_LOGGER, "Calculated Heading: " << headingBetweenCurrentAndNextPoint)

    flt64_t projectedLat;
    flt64_t projectedLon;
    arlcore::projectPositionOntoVector(in, &projectedLat, &projectedLon);

    // LOG
    std::stringstream projectedPointLog;
    projectedPointLog.precision(10);
    projectedPointLog << "PROJECTED LATITUDE/LONGITUDE: " << projectedLat << "," << projectedLon;
    UMAA_LOG_DEBUG(util::SYSTEM_LOGGER, projectedPointLog.str())

    flt64_t latShiftValue = 0;
    // Going south should shift with a negative latitude. North should shift with positive
    if (floor(headingBetweenCurrentAndNextPoint) >= -90 && ceil(headingBetweenCurrentAndNextPoint < 90)) {
      latShiftValue = -latitudeDistanceToPoint / ratioOfHaversineToLeadPoint;
    } else {
      latShiftValue = latitudeDistanceToPoint / ratioOfHaversineToLeadPoint;
    }
    UMAA_LOG_DEBUG(util::SYSTEM_LOGGER, "LatShiftValue: " << latShiftValue)

    latitude = arlcore::calculateLatitudeShift(projectedLat, latShiftValue);

    flt64_t lonShiftValue = 0;
    if (ceil(headingBetweenCurrentAndNextPoint) < 0 && floor(headingBetweenCurrentAndNextPoint) >= -180) {
      lonShiftValue = -longitudeDistanceToPoint / ratioOfHaversineToLeadPoint;
    } else {
      lonShiftValue = longitudeDistanceToPoint / ratioOfHaversineToLeadPoint;
    }
    UMAA_LOG_DEBUG(util::SYSTEM_LOGGER, "LonShiftValue: " << lonShiftValue)
    longitude = arlcore::calculateLongitudeShift(projectedLat, projectedLon, lonShiftValue);

    // LOGS
    std::stringstream wayPointLog;
    wayPointLog.precision(10);
    wayPointLog << "WAYPOINT LATITUDE/LONGITUDE: " << in.latTargetDeg << "," << in.lonTargetDeg;
    UMAA_LOG_DEBUG(util::SYSTEM_LOGGER, wayPointLog.str())

    std::stringstream calculatedPointLog;
    calculatedPointLog.precision(10);
    calculatedPointLog << "CALCULATED LATITUDE/LONGITUDE: " << latitude << "," << longitude;
    UMAA_LOG_DEBUG(util::SYSTEM_LOGGER, calculatedPointLog.str())

    std::stringstream currentPointLog;
    currentPointLog.precision(10);
    currentPointLog << "CURRENT LATITUDE/LONGITUDE: " << in.latCurrentDeg << "," << in.lonCurrentDeg;
    UMAA_LOG_DEBUG(util::SYSTEM_LOGGER, currentPointLog.str())
  }

  arlcore::convertLatLonToNedWithEcefOrigin(
    latitude,
    longitude,
    wptAltitude,
    &wptNorthM,
    &wptEastM,
    &wptDownM,
    in.xPos,
    in.yPos,
    in.zPos);

  // angle from vehicle to way-point
  flt64_t vehicleToWaypointRad = atan2(wptEastM, wptNorthM);

  // rotate to bring waypoint along 0 degree line
  flt64_t waypoint_north_rotate_m =
      cos(-vehicleToWaypointRad) * wptNorthM -
      sin(-vehicleToWaypointRad) * wptEastM;
  flt64_t waypoint_east_rotate_m =
      sin(-vehicleToWaypointRad) * wptNorthM +
      cos(-vehicleToWaypointRad) * wptEastM;

  // yaw in the rotate frame
  flt64_t yawRotateRad = arlcore::Unwind(in.yawTargetRad - vehicleToWaypointRad);

  // determine which side to aim for
  flt64_t circleCrossWaypointRad;
  if (yawRotateRad < 0.0) {
    circleCrossWaypointRad = yawRotateRad - M_PI / 2.0;
  } else {
    circleCrossWaypointRad = yawRotateRad + M_PI / 2.0;
  }

  // calculate location of center of circle for turning radius
  flt64_t circleCenterNorthM =
      waypoint_north_rotate_m +
      in.radiusOfCurvatureM * cos(circleCrossWaypointRad);
  flt64_t circleCenterEastM =
      waypoint_east_rotate_m +
      in.radiusOfCurvatureM * sin(circleCrossWaypointRad);

  // calculate angle from vehicle to center of circle
  flt64_t vehicleToCircleRad =
      atan2(circleCenterEastM, circleCenterNorthM);

  // distance from vehicle to center of circle
  flt64_t vehicleToCircleRangeM =
      sqrt(circleCenterEastM * circleCenterEastM +
          circleCenterNorthM * circleCenterNorthM);

  // should check for vehicle_to_circle_range_m > in.radiusOfCurvatureM
  // to prevent asin error.
  flt64_t vehicleToTangentPointRad = 0.0;

  if (vehicleToCircleRangeM > in.radiusOfCurvatureM) {
    if (yawRotateRad < 0.0) {
      vehicleToTangentPointRad =
          asin(in.radiusOfCurvatureM / vehicleToCircleRangeM);
    } else {
      vehicleToTangentPointRad =
          -asin(in.radiusOfCurvatureM / vehicleToCircleRangeM);
    }
  }

  flt64_t headToTransitPointRotatedRad =
      vehicleToCircleRad + vehicleToTangentPointRad;
  // Keeps heading between [-pi, pi]
  out.headingRad = arlcore::Unwind(headToTransitPointRotatedRad + vehicleToWaypointRad);
  UMAA_LOG_DEBUG(util::SYSTEM_LOGGER, "CALCULATED HEADING RADIANS: " << out.headingRad)
  return out;
}

//! \brief Creates a GlobalVectorCommand to reach the next waypoint
//! \param waypoint The current waypoint the vehicle is navigating towards
//! \param gpReport The current global pose report
//! \param previousWaypoint The waypoint we hit previously, optional if we are navigating towards the first waypoint
//! \return A GlobalVectorCommand if one could be created
static std::optional<GlobalVectorCommandType> calculateVectorCommandToReachWaypoint(
  const WaypointType& waypoint, const GlobalPoseReportType& gpReport,
  const std::optional<WaypointType>& previousWaypoint, int32_t leadDistanceMeters = 50) {
  GuidanceInput guidanceInput;

  flt64_t x;
  flt64_t y;
  flt64_t z;
  GeographicLib::Geocentric earth(GeographicLib::Constants::WGS84_a(),
                                  GeographicLib::Constants::WGS84_f());

  flt64_t altitude = 0;

  // Attitude needs to be set for this calculation. We need the yaw value.
  if (!waypoint.attitude().has_value()) {
    UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "No attitude set in waypoint data")
    return {};
  }

  earth.Forward(gpReport.position().geodeticLatitude(),
                gpReport.position().geodeticLongitude(),
                altitude,
                x, y, z);

  guidanceInput.xPos = x;
  guidanceInput.yPos = y;
  guidanceInput.zPos = z;
  guidanceInput.latTargetDeg = waypoint.position().geodeticLatitude();
  guidanceInput.lonTargetDeg = waypoint.position().geodeticLongitude();
  guidanceInput.yawTargetRad = waypoint.attitude().value().yawZ().yaw().yaw();

  guidanceInput.leadDistanceMeters = leadDistanceMeters;

  // If there is no previous waypoint, we cannot project a vector of our current position to the desired line.
  if (!previousWaypoint.has_value()) {
    guidanceInput.guideToLine = false;
    UMAA_LOG_DEBUG(util::SYSTEM_LOGGER, "Guide to Point")
  } else {
    UMAA_LOG_DEBUG(util::SYSTEM_LOGGER, "Guide to line")
    guidanceInput.guideToLine = true;
    guidanceInput.latCurrentDeg = gpReport.position().geodeticLatitude();
    guidanceInput.lonCurrentDeg = gpReport.position().geodeticLongitude();
    guidanceInput.latPrevTargetDeg = previousWaypoint.value().position().geodeticLatitude();
    guidanceInput.lonPrevTargetDeg = previousWaypoint.value().position().geodeticLongitude();
  }

  guidanceInput.radiusOfCurvatureM = 25;

  auto guidanceOutput = calculateGuidanceOutput(guidanceInput);

  if (!guidanceOutput) {
    UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Guidance output doesn't exist")
    return std::nullopt;
  }

  GlobalVectorCommandType gvCmd;
  gvCmd.direction().DirectionRequirementVariantTypeSubtypes()
    .DirectionTrueNorthRequirementVariantVariant(UMAA::Common::Orientation::DirectionTrueNorthRequirementVariantType());
  gvCmd.direction().DirectionRequirementVariantTypeSubtypes().DirectionTrueNorthRequirementVariantVariant().
    direction().direction(guidanceOutput.value().headingRad);

  gvCmd.speed().SpeedRequirementVariantTypeSubtypes()
    .GroundSpeedRequirementVariantVariant(UMAA::Common::Speed::GroundSpeedRequirementVariantType());
  // Using default speed of 1.0 if WaypointType speed is not provided
  if (!waypoint.speed()) {
    UMAA_LOG_INFO(util::SYSTEM_LOGGER, "Current WaypointType doesn't set speed. Using default speed of 1.0.")
    gvCmd.speed().SpeedRequirementVariantTypeSubtypes().GroundSpeedRequirementVariantVariant().speed()
      .speed(1.0);
  } else {
    gvCmd.speed().SpeedRequirementVariantTypeSubtypes().GroundSpeedRequirementVariantVariant().speed()
      .speed(waypoint.speed().value().SpeedVariantTypeSubtypes().GroundSpeedVariantVariant().speed());
  }

  // Check if the elevation is set or set elevation of the command to 0
  if (waypoint.elevation().has_value()) {
    gvCmd.elevation(waypoint.elevation().value());
  } else {
    UMAA::Common::Measurement::ElevationRequirementVariantType elevation;
    elevation.ElevationRequirementVariantTypeSubtypes()
      .DepthRequirementVariantVariant(UMAA::Common::Measurement::DepthRequirementVariantType());
    elevation.ElevationRequirementVariantTypeSubtypes().DepthRequirementVariantVariant().depth().depth(0);
    gvCmd.elevation(elevation);
  }

  gvCmd.directionMode(UMAA::Common::MaritimeEnumeration::DirectionModeEnumModule::DirectionModeEnumType::HEADING);
  gvCmd.timeStamp(arlcore::umaa::getTimestamp());

  return std::optional<GlobalVectorCommandType>(gvCmd);
}

//! \brief Checks if the vehicle has acheived the current waypoint by a Haversine Distance calculation
//! \param globalPoseReport The current Global Pose of the vehicle
//! \param currentWaypoint The current waypoint the vehicle is navigating towards
//! \return Boolean for if the waypoint was achieved or not
static bool wasWaypointAchieved(std::optional<GlobalPoseReportType> globalPoseReport,
    const WaypointType& currentWaypoint) {
  // Check if we have global pose
  if (!globalPoseReport) {
    return false;
  }

  flt64_t distanceFromWaypoint = arlcore::getHaversineDistance(
      globalPoseReport.value().position().geodeticLatitude(),
      globalPoseReport.value().position().geodeticLongitude(),
      currentWaypoint.position().geodeticLatitude(),
      currentWaypoint.position().geodeticLongitude());

  UMAA_LOG_TRACE(util::SYSTEM_LOGGER, "HAVERSINE DISTANCE VALUE: " << distanceFromWaypoint)

  if (distanceFromWaypoint <= currentWaypoint.captureRadius().distance()) {
    return true;
  }
  return false;
}

}  // namespace arl::algorithm
#endif  // INCLUDE_ALGORITHMS_GUIDANCE_GUIDANCE_H_
