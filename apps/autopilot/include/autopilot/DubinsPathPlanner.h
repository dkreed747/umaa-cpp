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

#ifndef APPS_AUTOPILOT_INCLUDE_AUTOPILOT_DUBINSPATHPLANNER_H_
#define APPS_AUTOPILOT_INCLUDE_AUTOPILOT_DUBINSPATHPLANNER_H_

#include <cstdint>
#include <vector>

#include <UMAA/MO/GlobalWaypointControl/GlobalWaypointType.hpp>
#include <UMAA/SA/GlobalPoseStatus/GlobalPoseReportType.hpp>

#include "ControlVector.h"
#include "ProgressTypes.h"

namespace arlcore::autopilot {

//! \brief Parameters that tune the planner. Capture defaults are used when a waypoint omits
//! its own tolerances.
struct PlannerParams {
  double turnRadiusM = 25.0;        // radius of curvature = speed / maxTurnRate
  double leadDistanceM = 50.0;      // carrot distance for guide-to-line following
  double posCaptureM = 10.0;        // default position capture radius
  double yawCaptureRad = 0.1745;    // default arrival-attitude capture half-width
  double elevCaptureM = 1.0;        // default elevation capture tolerance
  int maxMissesPerWaypoint = 3;     // misses before the route fails
  bool elevationCountsAsMiss = true;
  int maxReplans = 10;              // global guard against infinite spiraling
};

//! \brief A 2D horizontal path planner that smoothly drives a vehicle through a series of 3D
//! waypoints. Elevation/depth is passed through to the vehicle; horizontal guidance uses a
//! turn-circle / tangent-point arrival law (matured from the SDK's Guidance.h) so the vehicle
//! arrives at each waypoint on its commanded attitude. When a waypoint defines a track
//! tolerance the planner follows the straight line between waypoints (carrot chase, causing
//! crabbing when pushed off track); otherwise it flies a pure arrival-attitude approach.
//!
//! The route is planned up front; on every new navigation packet update() produces a fresh
//! control vector. Capture is monitored against position, arrival attitude, and elevation. A
//! miss triggers a replan from the current pose for the current and remaining waypoints, which
//! naturally produces a spiral when elevation cannot be met before the waypoint.
class DubinsPathPlanner {
 public:
  //! \brief Plan a route up front from the current pose through the given waypoints.
  void plan(const std::vector<UMAA::MO::GlobalWaypointControl::GlobalWaypointType>& waypoints,
            const UMAA::SA::GlobalPoseStatus::GlobalPoseReportType& start,
            const PlannerParams& params);

  //! \brief Produce a control vector for the current pose and advance capture/miss/replan
  //! state. Call once per navigation packet.
  ControlVector update(const UMAA::SA::GlobalPoseStatus::GlobalPoseReportType& pose);

  //! \brief Latest progress snapshot (mapped into the UMAA waypoint execution status report).
  const WaypointProgress& progress() const { return progress_; }

  bool routeComplete() const { return routeComplete_; }
  bool failed() const { return failed_; }
  bool hasRoute() const { return !waypoints_.empty(); }

 private:
  //! \brief Compute the desired heading (rad, true north) toward the current target.
  double computeHeading(const UMAA::SA::GlobalPoseStatus::GlobalPoseReportType& pose) const;

  //! \brief Compute the heading via the turn-circle / tangent-point arrival law.
  double computeArrivalHeading(const UMAA::SA::GlobalPoseStatus::GlobalPoseReportType& pose,
                               double targetLatDeg, double targetLonDeg, double targetYawRad) const;

  //! \brief Compute signed cross-track error (meters) from the prev->target line.
  double computeCrossTrackError(const UMAA::SA::GlobalPoseStatus::GlobalPoseReportType& pose) const;

  //! \brief Evaluate capture criteria for the current target against the pose.
  CaptureResult evaluateCapture(const UMAA::SA::GlobalPoseStatus::GlobalPoseReportType& pose) const;

  //! \brief Replan from the current pose for the current and remaining waypoints. Returns
  //! false if the replan budget (maxReplans) is exhausted.
  bool replanFromCurrent(const UMAA::SA::GlobalPoseStatus::GlobalPoseReportType& pose);

  //! \brief Update the cumulative/remaining distance metrics in progress_.
  void updateDistanceMetrics(const UMAA::SA::GlobalPoseStatus::GlobalPoseReportType& pose);

  std::vector<UMAA::MO::GlobalWaypointControl::GlobalWaypointType> waypoints_;
  std::vector<int> missCounts_;
  PlannerParams params_;
  std::size_t targetIndex_ = 0;
  double prevRefLatDeg_ = 0.0;   // line origin for the current segment (start/prev wp/replan pose)
  double prevRefLonDeg_ = 0.0;
  bool hasPrevRef_ = false;
  bool withinCaptureZone_ = false;
  bool routeComplete_ = false;
  bool failed_ = false;
  int replanCount_ = 0;
  WaypointProgress progress_;
  ControlVector lastVector_;
};

}  // namespace arlcore::autopilot
#endif  // APPS_AUTOPILOT_INCLUDE_AUTOPILOT_DUBINSPATHPLANNER_H_
