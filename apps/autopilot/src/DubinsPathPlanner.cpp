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
#include <limits>
#include <optional>
#include <vector>

#include "AngleMath.h"
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

//! \brief Azimuth (true north, clockwise) <-> math angle (+x east, counterclockwise).
//! The mapping is its own inverse.
double azToMath(double azRad) { return wrapPi(M_PI_2 - azRad); }
double mathToAz(double mathRad) { return wrapPi(M_PI_2 - mathRad); }

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

//! \brief Position capture tolerance for a waypoint (its own tolerance or the default).
double positionToleranceM(const GlobalWaypointType& wp, const PlannerParams& params) {
  if (wp.position().tolerance().has_value()) {
    return wp.position().tolerance().value().limit();
  }
  return params.posCaptureM;
}

}  // namespace

void DubinsPathPlanner::toLocal(double latDeg, double lonDeg, double* xE, double* yN) const {
  double z = 0.0;
  localFrame_.Forward(latDeg, lonDeg, 0.0, *xE, *yN, z);
}

Dubins2DPose DubinsPathPlanner::sampleExtended(const Leg& leg, double sM) {
  if (sM <= leg.dubinsLengthM) {
    return leg.path.sample(sM);
  }
  // Straight continuation covers both the final-approach runway (up to lengthM, ending at
  // the waypoint) and the fly-through extension beyond it.
  Dubins2DPose end = leg.path.sample(leg.dubinsLengthM);
  const double over = sM - leg.dubinsLengthM;
  end.x += over * std::cos(end.theta);
  end.y += over * std::sin(end.theta);
  return end;
}

double DubinsPathPlanner::arrivalAzimuth(std::size_t wpIndex, double fromXE, double fromYN) const {
  const GlobalWaypointType& wp = waypoints_[wpIndex];
  if (wp.attitude().has_value()) {
    return tolerance::extractYaw(wp.attitude().value()).yawRad;
  }
  // Natural fly-through heading: toward the next waypoint, or along the final approach for
  // the last one.
  double dE = 0.0;
  double dN = 0.0;
  if (wpIndex + 1 < waypoints_.size()) {
    dE = wpX_[wpIndex + 1] - wpX_[wpIndex];
    dN = wpY_[wpIndex + 1] - wpY_[wpIndex];
  } else {
    dE = wpX_[wpIndex] - fromXE;
    dN = wpY_[wpIndex] - fromYN;
  }
  if (std::hypot(dE, dN) < 1e-9) {
    return 0.0;
  }
  return std::atan2(dE, dN);
}

DubinsPathPlanner::Leg DubinsPathPlanner::buildLeg(const Dubins2DPose& startPose, std::size_t wpIndex) const {
  const double endAz = arrivalAzimuth(wpIndex, startPose.x, startPose.y);
  const double endTheta = azToMath(endAz);
  // Solve the curved portion to a virtual goal one turn radius short of the waypoint along
  // the arrival bearing; the leg then finishes with a straight runway through the waypoint.
  const double runwayM = params_.turnRadiusM;
  const Dubins2DPose virtualGoal{wpX_[wpIndex] - runwayM * std::cos(endTheta),
                                 wpY_[wpIndex] - runwayM * std::sin(endTheta), endTheta};
  std::optional<DubinsPath> path = DubinsPath::solve(startPose, virtualGoal, params_.turnRadiusM);
  if (!path.has_value()) {
    // solve() only fails on non-finite input; fall back to a degenerate straight run from a
    // sanitized origin so guidance can still make progress.
    path = DubinsPath::solve({0.0, 0.0, 0.0}, virtualGoal, 0.0);
  }
  return Leg(path.value(), runwayM, endAz);
}

void DubinsPathPlanner::plan(const std::vector<GlobalWaypointType>& waypoints,
                             const GlobalPoseReportType& start, const PlannerParams& params) {
  waypoints_ = waypoints;
  params_ = params;
  params_.sampleStepM = std::max(0.5, params_.sampleStepM);
  missCounts_.assign(waypoints_.size(), 0);
  targetIndex_ = 0;
  withinCaptureZone_ = false;
  routeComplete_ = waypoints_.empty();
  failed_ = false;
  replanCount_ = 0;
  hasLastPos_ = false;
  legProgressS_ = 0.0;
  currentLeg_.reset();
  progress_ = WaypointProgress{};
  progress_.valid = true;
  progress_.routeComplete = routeComplete_;
  progress_.waypointsRemaining = static_cast<int32_t>(waypoints_.size());
  lastVector_ = ControlVector{};

  localFrame_.Reset(poseLat(start), poseLon(start), 0.0);
  wpX_.resize(waypoints_.size());
  wpY_.resize(waypoints_.size());
  for (std::size_t i = 0; i < waypoints_.size(); i++) {
    toLocal(wpLat(waypoints_[i]), wpLon(waypoints_[i]), &wpX_[i], &wpY_[i]);
  }

  if (waypoints_.empty()) {
    return;
  }

  // The straight-line track origin for the first waypoint is the vehicle position at command
  // time (per the UMAA GlobalWaypointType trackTolerance semantics).
  segOriginXE_ = 0.0;
  segOriginYN_ = 0.0;
  const Dubins2DPose startPose{0.0, 0.0, azToMath(poseYaw(start))};
  currentLeg_ = buildLeg(startPose, 0);
  UMAA_LOG_INFO(util::SYSTEM_LOGGER, "DubinsPathPlanner planned route: " << waypoints_.size()
    << " waypoints, first leg " << currentLeg_->lengthM << " m (" << currentLeg_->path.word()
    << "), turn radius " << params_.turnRadiusM << " m")
}

CaptureResult DubinsPathPlanner::evaluateCapture(const GlobalPoseReportType& pose,
                                                 double distToWaypointM) const {
  CaptureResult result;
  const GlobalWaypointType& wp = waypoints_[targetIndex_];

  result.positionAchieved = distToWaypointM <= positionToleranceM(wp, params_);

  if (wp.attitude().has_value()) {
    const AttitudeValue att = tolerance::extractYaw(wp.attitude().value());
    result.attitudeAchieved = tolerance::attitudeAchieved(att, poseYaw(pose), params_.yawCaptureRad);
  }

  if (wp.elevation().has_value()) {
    const std::optional<ElevationValue> el = tolerance::extractElevation(wp.elevation().value());
    if (el.has_value()) {
      const std::optional<double> cur = poseElevation(pose, el->frame);
      result.elevationAchieved = cur.has_value() &&
          tolerance::elevationAchieved(el.value(), cur.value(), params_.elevCaptureM);
    } else {
      result.elevationAchieved = false;
    }
  }

  const bool attitudeOk = !result.attitudeAchieved.has_value() || result.attitudeAchieved.value();
  result.captured = result.positionAchieved && attitudeOk &&
                    (result.elevationAchieved || !params_.elevationCountsAsMiss);
  return result;
}

void DubinsPathPlanner::advanceToNextWaypoint() {
  segOriginXE_ = wpX_[targetIndex_];
  segOriginYN_ = wpY_[targetIndex_];
  const double arrivalAz = currentLeg_->endAzimuthRad;
  targetIndex_++;
  legProgressS_ = 0.0;
  withinCaptureZone_ = false;
  if (targetIndex_ >= waypoints_.size()) {
    routeComplete_ = true;
    currentLeg_.reset();
    UMAA_LOG_INFO(util::SYSTEM_LOGGER, "DubinsPathPlanner route complete")
    return;
  }
  const Dubins2DPose startPose{segOriginXE_, segOriginYN_, azToMath(arrivalAz)};
  currentLeg_ = buildLeg(startPose, targetIndex_);
  UMAA_LOG_INFO(util::SYSTEM_LOGGER, "DubinsPathPlanner advancing to waypoint " << targetIndex_
    << ", leg " << currentLeg_->lengthM << " m (" << currentLeg_->path.word() << ")")
}

void DubinsPathPlanner::registerMiss(const Dubins2DPose& current) {
  missCounts_[targetIndex_]++;
  UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Missed waypoint " << targetIndex_ << " (miss "
    << missCounts_[targetIndex_] << "/" << params_.maxMissesPerWaypoint << ")")
  if (missCounts_[targetIndex_] > params_.maxMissesPerWaypoint) {
    UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "DubinsPathPlanner exceeded miss budget for waypoint "
      << targetIndex_ << "; failing route")
    failed_ = true;
    return;
  }
  if (replanCount_ >= params_.maxReplans) {
    UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "DubinsPathPlanner exhausted replan budget; failing route")
    failed_ = true;
    return;
  }
  replanCount_++;
  // Replan the current leg from the live pose: the new Dubins solution loops back around to
  // the same arrival pose (a spiral when elevation is still being driven to target).
  currentLeg_ = buildLeg(current, targetIndex_);
  legProgressS_ = 0.0;
  segOriginXE_ = current.x;
  segOriginYN_ = current.y;
  withinCaptureZone_ = false;
  UMAA_LOG_INFO(util::SYSTEM_LOGGER, "DubinsPathPlanner replanned waypoint " << targetIndex_
    << " from live pose (replan " << replanCount_ << "/" << params_.maxReplans << "): leg "
    << currentLeg_->lengthM << " m (" << currentLeg_->path.word() << ")")
}

double DubinsPathPlanner::straightLineCrossTrackM(double xE, double yN) const {
  const double tE = wpX_[targetIndex_] - segOriginXE_;
  const double tN = wpY_[targetIndex_] - segOriginYN_;
  const double len = std::hypot(tE, tN);
  const double vE = xE - segOriginXE_;
  const double vN = yN - segOriginYN_;
  if (len < 1e-9) {
    return std::hypot(vE, vN);
  }
  // Positive = right (starboard) of the track direction.
  return (vE * tN - vN * tE) / len;
}

void DubinsPathPlanner::updateDistanceMetrics(double xE, double yN, double distToWaypointM) {
  double remaining = currentLeg_.has_value() ? std::max(0.0, currentLeg_->lengthM - legProgressS_)
                                             : distToWaypointM;
  for (std::size_t i = targetIndex_; i + 1 < waypoints_.size(); ++i) {
    remaining += std::hypot(wpX_[i + 1] - wpX_[i], wpY_[i + 1] - wpY_[i]);
  }
  progress_.distanceRemainingM = remaining;
  if (hasLastPos_) {
    progress_.cumulativeDistanceM += std::hypot(xE - lastXE_, yN - lastYN_);
  }
}

ControlVector DubinsPathPlanner::update(const GlobalPoseReportType& pose) {
  progress_.valid = true;
  if (waypoints_.empty() || routeComplete_ || failed_) {
    progress_.routeComplete = routeComplete_;
    progress_.failed = failed_;
    // Hold heading but stop driving once the route is over.
    ControlVector hold = lastVector_;
    hold.speedMps = 0.0;
    return hold;
  }

  double xE = 0.0;
  double yN = 0.0;
  toLocal(poseLat(pose), poseLon(pose), &xE, &yN);
  const Dubins2DPose current{xE, yN, azToMath(poseYaw(pose))};

  const GlobalWaypointType& wp = waypoints_[targetIndex_];
  const double dist = std::hypot(wpX_[targetIndex_] - xE, wpY_[targetIndex_] - yN);

  if (!currentLeg_.has_value()) {
    currentLeg_ = buildLeg(current, targetIndex_);
    legProgressS_ = 0.0;
  }
  const Leg& leg = currentLeg_.value();

  // Pure-pursuit lead distance: capped relative to the turn radius, because a lead much
  // longer than the turning circle cuts the corners of the final arc so hard the capture
  // zone is missed entirely.
  const double lead = std::clamp(std::min(params_.leadDistanceM, 1.5 * params_.turnRadiusM),
                                 2.0 * params_.sampleStepM, params_.leadDistanceM);
  const double overshootBudget = std::max(lead, 2.0 * positionToleranceM(wp, params_));

  // Advance the monotonic arc-length progress pointer: search a bounded window ahead of the
  // previous progress (with a small allowance backward) for the closest path sample. The
  // parametrization extends beyond the path end along the arrival heading so both the
  // progress pointer and the carrot keep moving through the waypoint (a pinned carrot at the
  // endpoint would make the vehicle orbit it forever).
  {
    const double step = params_.sampleStepM;
    const double back = std::min(legProgressS_, 2.0 * step);
    const double windowAheadM = std::max(4.0 * step, 3.0 * lead);
    const double sMax = leg.lengthM + overshootBudget + step;
    double bestS = legProgressS_;
    double bestD = std::numeric_limits<double>::max();
    for (double s = legProgressS_ - back; s <= std::min(legProgressS_ + windowAheadM, sMax);
         s += step) {
      const Dubins2DPose p = sampleExtended(leg, s);
      const double d = std::hypot(p.x - xE, p.y - yN);
      if (d < bestD) {
        bestD = d;
        bestS = s;
      }
    }
    legProgressS_ = bestS;
  }

  // Carrot at the lead distance along the (extended) path. The lead tightens on final
  // approach so arc-cutting error at arrival stays inside the capture radius; the floor
  // (a fraction of the turn radius) keeps the pursuit stable for a rate-limited vehicle.
  const double remainingM = std::max(0.0, leg.lengthM - legProgressS_);
  const double carrotLead = std::max(std::min(lead, 0.7 * remainingM + params_.sampleStepM),
                                     std::max(0.8 * params_.turnRadiusM, 2.0 * params_.sampleStepM));
  const Dubins2DPose carrot = sampleExtended(leg, legProgressS_ + carrotLead);
  const double carrotX = carrot.x;
  const double carrotY = carrot.y;

  ControlVector cv;
  const double dxE = carrotX - xE;
  const double dyN = carrotY - yN;
  cv.headingRad = (std::hypot(dxE, dyN) > 1e-9) ? std::atan2(dxE, dyN) : poseYaw(pose);
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

  // Progress reporting.
  const CaptureResult cap = evaluateCapture(pose, dist);
  progress_.distanceToWaypointM = dist;
  progress_.positionAchieved = cap.positionAchieved;
  progress_.attitudeAchieved = cap.attitudeAchieved;
  progress_.elevationAchieved = cap.elevationAchieved;
  progress_.waypointId = arlcore::NumericGuid(wp.waypointID());
  progress_.waypointsRemaining = static_cast<int32_t>(waypoints_.size() - targetIndex_);
  if (wp.trackTolerance().has_value()) {
    const double xte = straightLineCrossTrackM(xE, yN);
    progress_.crossTrackErrorM = xte;
    const std::optional<double> tol = tolerance::extractTrackToleranceM(wp.trackTolerance().value());
    progress_.trackLineAchieved = !tol.has_value() || std::fabs(xte) <= tol.value();
  } else {
    progress_.crossTrackErrorM.reset();
    progress_.trackLineAchieved = true;
  }
  updateDistanceMetrics(xE, yN, dist);
  lastXE_ = xE;
  lastYN_ = yN;
  hasLastPos_ = true;

  // Capture / miss state machine: capture is evaluated continuously while inside the capture
  // zone; leaving the zone (or overflying the planned path) without a clean capture is a miss.
  const bool inZone = dist <= positionToleranceM(wp, params_);
  if (inZone) {
    withinCaptureZone_ = true;
    if (cap.captured) {
      advanceToNextWaypoint();
      if (routeComplete_) {
        cv.speedMps = 0.0;
        lastVector_ = cv;
        progress_.routeComplete = true;
        progress_.waypointsRemaining = 0;
      }
    }
  } else if (withinCaptureZone_) {
    withinCaptureZone_ = false;
    // Falling edge: exited the capture zone without capturing. Only count it as a miss when
    // it happened on final approach — in dense waypoint fields (e.g. lawnmower lanes spaced
    // tighter than the turning circle) the planned path legitimately crosses the target's
    // capture zone mid-turn, and the path itself will bring the vehicle back for arrival.
    if (legProgressS_ > leg.lengthM - (lead + 2.0 * positionToleranceM(wp, params_))) {
      registerMiss(current);
    }
  } else if (legProgressS_ > leg.lengthM + overshootBudget - 1e-9) {
    // Overflew the end of the planned path without ever entering the capture zone (the
    // capture radius is smaller than our tracking error): loop back around.
    registerMiss(current);
  }
  progress_.failed = failed_;
  progress_.routeComplete = routeComplete_;

  return cv;
}

}  // namespace arlcore::autopilot
