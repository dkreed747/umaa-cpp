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

#include "DubinsPathPlanner.h"

#include <algorithm>
#include <cmath>
#include <optional>

#include <GeographicLib/Geocentric.hpp>

#include "GeographicUtils.h"
#include "Logger.h"
#include "ToleranceUtils.h"

namespace arlcore::autopilot {

using UMAA::MO::GlobalWaypointControl::GlobalWaypointType;
using UMAA::SA::GlobalPoseStatus::GlobalPoseReportType;

namespace {

double poseLat(const GlobalPoseReportType& p) { return p.position().geodeticLatitude(); }
double poseLon(const GlobalPoseReportType& p) { return p.position().geodeticLongitude(); }
double poseYaw(const GlobalPoseReportType& p) { return p.attitude().yaw().yaw(); }

double wpLat(const GlobalWaypointType& w) { return w.position().value().geodeticLatitude(); }
double wpLon(const GlobalWaypointType& w) { return w.position().value().geodeticLongitude(); }

//! \brief Current pose elevation in the requested frame, if available.
std::optional<double> poseElevation(const GlobalPoseReportType& p, ElevationFrame frame) {
  switch (frame) {
    case ElevationFrame::DEPTH:
      return p.depth().has_value() ? std::optional<double>(p.depth().value()) : std::nullopt;
    case ElevationFrame::ALTITUDE_MSL:
      return p.altitude().has_value() ? std::optional<double>(p.altitude().value()) : std::nullopt;
    case ElevationFrame::ALTITUDE_AGL:
      return p.altitudeAGL().has_value() ? std::optional<double>(p.altitudeAGL().value()) : std::nullopt;
    case ElevationFrame::ALTITUDE_GEODETIC:
      return p.altitudeGeodetic().has_value() ? std::optional<double>(p.altitudeGeodetic().value()) : std::nullopt;
    default:
      return std::nullopt;
  }
}

}  // namespace

void DubinsPathPlanner::plan(const std::vector<GlobalWaypointType>& waypoints,
                             const GlobalPoseReportType& start, const PlannerParams& params) {
  waypoints_ = waypoints;
  params_ = params;
  missCounts_.assign(waypoints_.size(), 0);
  targetIndex_ = 0;
  // The line origin for the first waypoint is the vehicle position at command time (per the
  // UMAA GlobalWaypointType trackTolerance semantics).
  prevRefLatDeg_ = poseLat(start);
  prevRefLonDeg_ = poseLon(start);
  hasPrevRef_ = true;
  withinCaptureZone_ = false;
  routeComplete_ = waypoints_.empty();
  failed_ = false;
  replanCount_ = 0;
  progress_ = WaypointProgress{};
  progress_.valid = true;
  progress_.routeComplete = routeComplete_;
}

double DubinsPathPlanner::computeArrivalHeading(const GlobalPoseReportType& pose, double targetLatDeg,
                                                double targetLonDeg, double targetYawRad) const {
  // Turn-circle / tangent-point arrival law (matured from Guidance.h calculateGuidanceOutput).
  GeographicLib::Geocentric earth(GeographicLib::Constants::WGS84_a(), GeographicLib::Constants::WGS84_f());
  double x = 0.0;
  double y = 0.0;
  double z = 0.0;
  earth.Forward(poseLat(pose), poseLon(pose), 0.0, x, y, z);

  double north = 0.0;
  double east = 0.0;
  double down = 0.0;
  arlcore::convertLatLonToNedWithEcefOrigin(targetLatDeg, targetLonDeg, 0.0, &north, &east, &down, x, y, z);

  const double vehicleToWaypointRad = std::atan2(east, north);
  const double northRot = std::cos(-vehicleToWaypointRad) * north - std::sin(-vehicleToWaypointRad) * east;
  const double eastRot = std::sin(-vehicleToWaypointRad) * north + std::cos(-vehicleToWaypointRad) * east;

  const double yawRotateRad = arlcore::Unwind(targetYawRad - vehicleToWaypointRad);
  const double circleCrossRad = (yawRotateRad < 0.0) ? yawRotateRad - M_PI / 2.0 : yawRotateRad + M_PI / 2.0;

  const double centerNorth = northRot + params_.turnRadiusM * std::cos(circleCrossRad);
  const double centerEast = eastRot + params_.turnRadiusM * std::sin(circleCrossRad);

  const double vehicleToCircleRad = std::atan2(centerEast, centerNorth);
  const double range = std::sqrt(centerNorth * centerNorth + centerEast * centerEast);

  double vehicleToTangentRad = 0.0;
  if (range > params_.turnRadiusM) {
    vehicleToTangentRad = (yawRotateRad < 0.0) ? std::asin(params_.turnRadiusM / range)
                                               : -std::asin(params_.turnRadiusM / range);
  }

  const double headRotated = vehicleToCircleRad + vehicleToTangentRad;
  return arlcore::Unwind(headRotated + vehicleToWaypointRad);
}

double DubinsPathPlanner::computeCrossTrackError(const GlobalPoseReportType& pose) const {
  if (!hasPrevRef_ || targetIndex_ >= waypoints_.size()) {
    return 0.0;
  }
  arl::algorithm::GuidanceInput in;
  in.latCurrentDeg = poseLat(pose);
  in.lonCurrentDeg = poseLon(pose);
  in.latTargetDeg = wpLat(waypoints_[targetIndex_]);
  in.lonTargetDeg = wpLon(waypoints_[targetIndex_]);
  in.latPrevTargetDeg = prevRefLatDeg_;
  in.lonPrevTargetDeg = prevRefLonDeg_;

  double projLat = 0.0;
  double projLon = 0.0;
  arlcore::projectPositionOntoVector(in, &projLat, &projLon);
  return arlcore::getHaversineDistance(in.latCurrentDeg, in.lonCurrentDeg, projLat, projLon);
}

double DubinsPathPlanner::computeHeading(const GlobalPoseReportType& pose) const {
  const GlobalWaypointType& wp = waypoints_[targetIndex_];
  const double tgtLat = wpLat(wp);
  const double tgtLon = wpLon(wp);

  // Guide-to-line (carrot chase) when a track tolerance is defined: hold the straight segment
  // between the previous reference and the target. A tighter tolerance shortens the lead
  // distance, producing tighter tracking (and crabbing when pushed off the line).
  if (wp.trackTolerance().has_value() && hasPrevRef_) {
    const std::optional<double> trackTolM = tolerance::extractTrackToleranceM(wp.trackTolerance().value());
    double lead = params_.leadDistanceM;
    if (trackTolM.has_value()) {
      lead = std::clamp(trackTolM.value() * 5.0, 5.0, params_.leadDistanceM);
    }

    arl::algorithm::GuidanceInput in;
    in.latCurrentDeg = poseLat(pose);
    in.lonCurrentDeg = poseLon(pose);
    in.latTargetDeg = tgtLat;
    in.lonTargetDeg = tgtLon;
    in.latPrevTargetDeg = prevRefLatDeg_;
    in.lonPrevTargetDeg = prevRefLonDeg_;

    double projLat = 0.0;
    double projLon = 0.0;
    arlcore::projectPositionOntoVector(in, &projLat, &projLon);

    const double segBearingRad = arlcore::azimuthBetweenPoints(prevRefLatDeg_, prevRefLonDeg_, tgtLat, tgtLon);
    const UMAA::Common::Measurement::GeoPosition2D carrot =
        arlcore::vincentyDirect(UMAA::Common::Measurement::GeoPosition2D(projLat, projLon), segBearingRad, lead);
    return arlcore::Unwind(arlcore::azimuthBetweenPoints(poseLat(pose), poseLon(pose),
        carrot.geodeticLatitude(), carrot.geodeticLongitude()));
  }

  // Pure arrival-attitude approach. Honor the waypoint's arrival yaw if provided, otherwise
  // simply steer directly at the point.
  if (wp.attitude().has_value()) {
    const AttitudeValue att = tolerance::extractYaw(wp.attitude().value());
    return computeArrivalHeading(pose, tgtLat, tgtLon, att.yawRad);
  }
  return arlcore::Unwind(arlcore::azimuthBetweenPoints(poseLat(pose), poseLon(pose), tgtLat, tgtLon));
}

CaptureResult DubinsPathPlanner::evaluateCapture(const GlobalPoseReportType& pose) const {
  CaptureResult result;
  const GlobalWaypointType& wp = waypoints_[targetIndex_];

  double posTol = params_.posCaptureM;
  if (wp.position().tolerance().has_value()) {
    posTol = wp.position().tolerance().value().limit();
  }
  const double dist = arlcore::getHaversineDistance(poseLat(pose), poseLon(pose), wpLat(wp), wpLon(wp));
  result.positionAchieved = dist <= posTol;

  if (wp.attitude().has_value()) {
    const AttitudeValue att = tolerance::extractYaw(wp.attitude().value());
    const double yawTol = att.yawToleranceRad.value_or(params_.yawCaptureRad);
    const double yawErr = std::fabs(arlcore::Unwind(poseYaw(pose) - att.yawRad));
    result.attitudeAchieved = yawErr <= yawTol;
  }

  if (wp.elevation().has_value()) {
    const std::optional<ElevationValue> el = tolerance::extractElevation(wp.elevation().value());
    if (el.has_value()) {
      const std::optional<double> cur = poseElevation(pose, el->frame);
      const double elevTol = el->toleranceM.value_or(params_.elevCaptureM);
      result.elevationAchieved = cur.has_value() && std::fabs(cur.value() - el->valueM) <= elevTol;
    } else {
      result.elevationAchieved = false;
    }
  }

  bool attitudeOk = !result.attitudeAchieved.has_value() || result.attitudeAchieved.value();
  result.captured = result.positionAchieved && attitudeOk &&
                    (result.elevationAchieved || !params_.elevationCountsAsMiss);
  return result;
}

bool DubinsPathPlanner::replanFromCurrent(const GlobalPoseReportType& pose) {
  if (replanCount_ >= params_.maxReplans) {
    UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "DubinsPathPlanner exhausted replan budget; failing route")
    return false;
  }
  replanCount_++;
  // Reset the segment origin to the current pose so the arrival law loops back around to the
  // same waypoint attitude (a spiral up/down while elevation is driven toward target).
  prevRefLatDeg_ = poseLat(pose);
  prevRefLonDeg_ = poseLon(pose);
  hasPrevRef_ = true;
  withinCaptureZone_ = false;
  UMAA_LOG_INFO(util::SYSTEM_LOGGER, "DubinsPathPlanner replanning waypoint " << targetIndex_
    << " (replan " << replanCount_ << "/" << params_.maxReplans << ")")
  return true;
}

void DubinsPathPlanner::updateDistanceMetrics(const GlobalPoseReportType& pose) {
  if (targetIndex_ >= waypoints_.size()) {
    progress_.distanceRemainingM = 0.0;
    return;
  }
  double remaining = arlcore::getHaversineDistance(poseLat(pose), poseLon(pose),
      wpLat(waypoints_[targetIndex_]), wpLon(waypoints_[targetIndex_]));
  for (std::size_t i = targetIndex_; i + 1 < waypoints_.size(); ++i) {
    remaining += arlcore::getHaversineDistance(wpLat(waypoints_[i]), wpLon(waypoints_[i]),
        wpLat(waypoints_[i + 1]), wpLon(waypoints_[i + 1]));
  }
  progress_.distanceRemainingM = remaining;
}

ControlVector DubinsPathPlanner::update(const GlobalPoseReportType& pose) {
  progress_.valid = true;
  if (waypoints_.empty() || routeComplete_ || failed_) {
    progress_.routeComplete = routeComplete_;
    progress_.failed = failed_;
    return lastVector_;
  }

  const GlobalWaypointType& wp = waypoints_[targetIndex_];

  ControlVector cv;
  cv.headingRad = computeHeading(pose);
  const std::optional<SpeedValue> sp = tolerance::extractSpeed(wp.speed());
  cv.speedMps = sp.has_value() ? sp->speedMps : 0.0;
  if (wp.elevation().has_value()) {
    const std::optional<ElevationValue> el = tolerance::extractElevation(wp.elevation().value());
    if (el.has_value()) {
      cv.elevationM = el->valueM;
      cv.elevationFrame = el->frame;
    }
  }
  lastVector_ = cv;

  const CaptureResult cap = evaluateCapture(pose);
  const double dist = arlcore::getHaversineDistance(poseLat(pose), poseLon(pose), wpLat(wp), wpLon(wp));

  progress_.distanceToWaypointM = dist;
  progress_.positionAchieved = cap.positionAchieved;
  progress_.attitudeAchieved = cap.attitudeAchieved;
  progress_.elevationAchieved = cap.elevationAchieved;
  progress_.waypointId = arlcore::NumericGuid(wp.waypointID());
  progress_.waypointsRemaining = static_cast<int32_t>(waypoints_.size() - targetIndex_);
  if (wp.trackTolerance().has_value()) {
    const double xte = computeCrossTrackError(pose);
    progress_.crossTrackErrorM = xte;
    const double tol = tolerance::extractTrackToleranceM(wp.trackTolerance().value()).value_or(params_.posCaptureM);
    progress_.trackLineAchieved = xte <= tol;
  } else {
    progress_.crossTrackErrorM.reset();
    progress_.trackLineAchieved = true;
  }
  updateDistanceMetrics(pose);

  // Capture evaluation on the rising edge of entering the position capture zone.
  double posTol = params_.posCaptureM;
  if (wp.position().tolerance().has_value()) {
    posTol = wp.position().tolerance().value().limit();
  }
  const bool inZone = dist <= posTol;
  if (inZone && !withinCaptureZone_) {
    const bool attitudeOk = !cap.attitudeAchieved.has_value() || cap.attitudeAchieved.value();
    const bool cleanCapture = cap.positionAchieved && attitudeOk &&
                              (cap.elevationAchieved || !params_.elevationCountsAsMiss);
    if (cleanCapture) {
      // Advance to the next waypoint; the captured waypoint becomes the next segment origin.
      prevRefLatDeg_ = wpLat(wp);
      prevRefLonDeg_ = wpLon(wp);
      targetIndex_++;
      withinCaptureZone_ = false;
      if (targetIndex_ >= waypoints_.size()) {
        routeComplete_ = true;
        progress_.routeComplete = true;
        progress_.waypointsRemaining = 0;
      }
    } else {
      missCounts_[targetIndex_]++;
      UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Missed waypoint " << targetIndex_ << " (miss "
        << missCounts_[targetIndex_] << "/" << params_.maxMissesPerWaypoint << ")")
      if (missCounts_[targetIndex_] > params_.maxMissesPerWaypoint || !replanFromCurrent(pose)) {
        failed_ = true;
        progress_.failed = true;
      }
    }
  } else {
    withinCaptureZone_ = inZone;
  }

  return cv;
}

}  // namespace arlcore::autopilot
