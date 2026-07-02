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
#include <optional>
#include <vector>

#include <GeographicLib/LocalCartesian.hpp>

#include <UMAA/MO/GlobalWaypointControl/GlobalWaypointType.hpp>
#include <UMAA/SA/GlobalPoseStatus/GlobalPoseReportType.hpp>

#include "ControlVector.h"
#include "DubinsPath.h"
#include "ProgressTypes.h"

namespace arlcore::autopilot {

//! \brief Parameters that tune the planner. Capture defaults are used when a waypoint omits
//! its own tolerances.
struct PlannerParams {
  double turnRadiusM = 25.0;        // radius of curvature = speed / maxTurnRate
  double leadDistanceM = 50.0;      // pure-pursuit carrot distance along the planned path
  double posCaptureM = 10.0;        // default position capture radius
  double yawCaptureRad = 0.1745;    // default arrival-attitude capture half-width
  double elevCaptureM = 1.0;        // default elevation capture tolerance
  int maxMissesPerWaypoint = 3;     // misses before the route fails
  bool elevationCountsAsMiss = true;
  int maxReplans = 10;              // global guard against infinite spiraling
  double sampleStepM = 2.0;         // path polyline sampling resolution
};

//! \brief A horizontal path planner that drives a vehicle through a series of 3D waypoints
//! along true Dubins paths. For every leg (previous waypoint or the plan/replan pose, to the
//! next waypoint) the shortest curvature-bounded Dubins path is solved (all six words, see
//! DubinsPath) using the platform turn radius. The vehicle follows the planned path with a
//! pure-pursuit carrot at the configured lead distance, so it arrives at each waypoint on its
//! commanded attitude when one is required; waypoints without an attitude requirement get a
//! natural fly-through heading toward the following waypoint. Elevation/depth and speed are
//! passed through to the vehicle per-waypoint.
//!
//! Capture is monitored continuously while the vehicle is inside the position-capture zone
//! (position + arrival attitude + elevation criteria). Leaving the zone without a clean
//! capture — or overflying the end of the planned path without ever entering the zone —
//! counts as a miss and triggers a replan of the current leg from the live pose (which
//! naturally produces a loop-back or spiral when elevation cannot be met in time), bounded by
//! maxMissesPerWaypoint and maxReplans. When the route completes the planner commands zero
//! speed while holding the final heading.
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
  //! \brief One planned leg: the Dubins path from the leg start pose to its target waypoint.
  struct Leg {
    Leg(const DubinsPath& p, double endAz) : path(p), lengthM(p.lengthM()), endAzimuthRad(endAz) {}
    DubinsPath path;      // in the local tangent plane (math convention)
    double lengthM;
    double endAzimuthRad;  // arrival azimuth at the waypoint (true-north, [-pi, pi])
  };

  //! \brief Convert a geodetic position to the local tangent plane (x east, y north).
  void toLocal(double latDeg, double lonDeg, double* xE, double* yN) const;

  //! \brief Sample the leg's path at arc length s, extending past the end along the arrival
  //! heading so guidance keeps flowing through the waypoint.
  static Dubins2DPose sampleExtended(const Leg& leg, double sM);

  //! \brief Build the Dubins leg from a local start pose to waypoint `wpIndex`.
  Leg buildLeg(const Dubins2DPose& startPose, std::size_t wpIndex) const;

  //! \brief The commanded arrival azimuth for waypoint `wpIndex` (attitude requirement if
  //! present, otherwise a natural fly-through heading toward the next waypoint).
  double arrivalAzimuth(std::size_t wpIndex, double fromXE, double fromYN) const;

  //! \brief Evaluate capture criteria for the current target against the pose.
  CaptureResult evaluateCapture(const UMAA::SA::GlobalPoseStatus::GlobalPoseReportType& pose,
                                double distToWaypointM) const;

  //! \brief Advance to the next waypoint after a clean capture.
  void advanceToNextWaypoint();

  //! \brief Register a miss on the current waypoint; replans the leg from the live pose.
  //! Marks the route failed when the miss/replan budget is exhausted.
  void registerMiss(const Dubins2DPose& current);

  //! \brief Update the distance metrics in progress_ for the current vehicle position.
  void updateDistanceMetrics(double xE, double yN, double distToWaypointM);

  //! \brief Signed cross-track error from the straight line between the segment origin and
  //! the target waypoint (per UMAA track-tolerance semantics). Positive = right of track.
  double straightLineCrossTrackM(double xE, double yN) const;

  std::vector<UMAA::MO::GlobalWaypointControl::GlobalWaypointType> waypoints_;
  std::vector<int> missCounts_;
  PlannerParams params_;

  GeographicLib::LocalCartesian localFrame_;  // origin at the plan start pose
  std::vector<double> wpX_;  // waypoint local coordinates (east)
  std::vector<double> wpY_;  // waypoint local coordinates (north)

  std::size_t targetIndex_ = 0;
  std::optional<Leg> currentLeg_;
  double legProgressS_ = 0.0;          // monotonic arc-length progress along the current leg
  double segOriginXE_ = 0.0;           // straight-line track origin for cross-track reporting
  double segOriginYN_ = 0.0;
  bool withinCaptureZone_ = false;
  bool routeComplete_ = false;
  bool failed_ = false;
  int replanCount_ = 0;
  bool hasLastPos_ = false;
  double lastXE_ = 0.0;                // previous update position (cumulative distance)
  double lastYN_ = 0.0;
  WaypointProgress progress_;
  ControlVector lastVector_;
};

}  // namespace arlcore::autopilot
#endif  // APPS_AUTOPILOT_INCLUDE_AUTOPILOT_DUBINSPATHPLANNER_H_
