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

#include "AutopilotBrain.h"

#include <cmath>
#include <optional>

#include "GeographicUtils.h"
#include "Logger.h"
#include "ToleranceUtils.h"

namespace arlcore::autopilot {

using UMAA::SA::GlobalPoseStatus::GlobalPoseReportType;

namespace {

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

AutopilotBrain::AutopilotBrain(NavState* nav, IVehicleControl* vehicle, const AutopilotConfig& config) :
    nav_(nav),
    vehicle_(vehicle),
    config_(config),
    arbiter_(config.arbitration.vectorPriority, config.arbitration.waypointPriority) {}

PlannerParams AutopilotBrain::derivePlannerParams() const {
  PlannerParams p;
  p.leadDistanceM = config_.planner.leadDistanceM;
  p.posCaptureM = config_.waypointTolerances.positionM;
  p.yawCaptureRad = config_.waypointTolerances.yawRad;
  p.elevCaptureM = config_.waypointTolerances.elevationM;
  p.maxMissesPerWaypoint = config_.planner.maxMissesPerWaypoint;
  p.elevationCountsAsMiss = config_.planner.elevationCountsAsMiss;
  p.maxReplans = config_.planner.maxReplans;

  // Turn radius = representative speed / max turn rate, from the surface capabilities.
  const CapabilityLimits& surf = config_.platformCapabilities.surface;
  const std::optional<double> speed = surf.cruisingSpeedMps.has_value() ? surf.cruisingSpeedMps
                                                                        : surf.maxForwardSpeedMps;
  p.turnRadiusM = config_.planner.defaultRadiusOfCurvatureM;
  if (speed.has_value() && surf.maxTurnRateRps.has_value() && surf.maxTurnRateRps.value() > 0.0) {
    p.turnRadiusM = speed.value() / surf.maxTurnRateRps.value();
  }
  return p;
}

void AutopilotBrain::setVectorSetpoint(
    const UMAA::MO::GlobalVectorControl::GlobalVectorCommandType& cmd) {
  std::lock_guard<std::mutex> lock(mtx_);
  activeVector_ = cmd;
  mode_ = DriveSource::VECTOR;
  vectorProgress_ = VectorProgress{};
  UMAA_LOG_INFO(util::SYSTEM_LOGGER, "Autopilot brain: vector setpoint installed")
}

bool AutopilotBrain::setWaypointSetpoint(
    const std::vector<UMAA::MO::GlobalWaypointControl::GlobalWaypointType>& waypoints) {
  std::lock_guard<std::mutex> lock(mtx_);
  const std::optional<GlobalPoseReportType> pose = nav_->pose();
  if (!pose.has_value()) {
    UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Cannot plan waypoint route without a navigation fix")
    return false;
  }
  planner_.plan(waypoints, pose.value(), derivePlannerParams());
  waypointProgress_ = planner_.progress();
  mode_ = DriveSource::WAYPOINT;
  UMAA_LOG_INFO(util::SYSTEM_LOGGER, "Autopilot brain: waypoint route installed ("
    << waypoints.size() << " waypoints)")
  return true;
}

void AutopilotBrain::clearSetpoint(DriveSource src) {
  std::lock_guard<std::mutex> lock(mtx_);
  if (mode_ == src) {
    mode_ = DriveSource::NONE;
    UMAA_LOG_INFO(util::SYSTEM_LOGGER, "Autopilot brain: setpoint cleared")
  }
}

void AutopilotBrain::onNavUpdate() {
  std::lock_guard<std::mutex> lock(mtx_);
  const std::optional<GlobalPoseReportType> pose = nav_->pose();
  if (!pose.has_value()) {
    return;
  }
  switch (mode_) {
    case DriveSource::VECTOR:
      updateVectorControl(pose.value());
      break;
    case DriveSource::WAYPOINT:
      updateWaypointControl(pose.value());
      break;
    default:
      break;
  }
}

void AutopilotBrain::updateVectorControl(const GlobalPoseReportType& pose) {
  ControlVector cv;
  const double poseYaw = pose.attitude().yaw().yaw();

  const std::optional<DirectionValue> dir = tolerance::extractDirection(activeVector_.direction());
  cv.headingRad = dir.has_value() ? dir->headingRad : poseYaw;

  const std::optional<SpeedValue> sp = tolerance::extractSpeed(activeVector_.speed());
  cv.speedMps = sp.has_value() ? sp->speedMps : 0.0;

  std::optional<ElevationValue> elev;
  if (activeVector_.elevation().has_value()) {
    elev = tolerance::extractElevation(activeVector_.elevation().value());
    if (elev.has_value()) {
      cv.elevationM = elev->valueM;
      cv.elevationFrame = elev->frame;
    }
  }

  vehicle_->sendControlVector(cv);

  // Achieved-flag evaluation against the commanded tolerances (or configured defaults).
  VectorProgress prog;
  prog.valid = true;
  const double dirTol = (dir.has_value() && dir->toleranceRad.has_value()) ? dir->toleranceRad.value()
                                                                           : config_.vectorTolerances.directionRad;
  prog.directionAchieved = dir.has_value() &&
      std::fabs(arlcore::Unwind(poseYaw - cv.headingRad)) <= dirTol;

  const double speedTol = (sp.has_value() && sp->toleranceMps.has_value()) ? sp->toleranceMps.value()
                                                                           : config_.vectorTolerances.speedMps;
  prog.speedAchieved = std::fabs(nav_->groundSpeedMps() - cv.speedMps) <= speedTol;

  if (elev.has_value()) {
    const std::optional<double> cur = poseElevation(pose, elev->frame);
    const double elevTol = elev->toleranceM.value_or(config_.vectorTolerances.elevationM);
    prog.elevationAchieved = cur.has_value() && std::fabs(cur.value() - elev->valueM) <= elevTol;
  } else {
    prog.elevationAchieved = true;
  }

  vectorProgress_ = prog;
}

void AutopilotBrain::updateWaypointControl(const GlobalPoseReportType& pose) {
  const ControlVector cv = planner_.update(pose);
  vehicle_->sendControlVector(cv);

  WaypointProgress prog = planner_.progress();
  // The planner does not see speed; evaluate speed achievement here from the nav fix.
  prog.speedAchieved = std::fabs(nav_->groundSpeedMps() - cv.speedMps) <= config_.vectorTolerances.speedMps;
  waypointProgress_ = prog;
}

VectorProgress AutopilotBrain::vectorProgress() const {
  std::lock_guard<std::mutex> lock(mtx_);
  return vectorProgress_;
}

WaypointProgress AutopilotBrain::waypointProgress() const {
  std::lock_guard<std::mutex> lock(mtx_);
  return waypointProgress_;
}

DriveSource AutopilotBrain::mode() const {
  std::lock_guard<std::mutex> lock(mtx_);
  return mode_;
}

}  // namespace arlcore::autopilot
