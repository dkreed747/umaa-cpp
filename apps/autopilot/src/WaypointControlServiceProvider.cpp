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

#include "WaypointControlServiceProvider.h"

#include <optional>

#include "LargeList.h"
#include "Logger.h"
#include "ToleranceUtils.h"
#include "UmaaUtils.h"

namespace arlcore::autopilot {

using arlcore::umaa::services::CommandStateResult;
using arlcore::umaa::services::IncomingCommandBehavior;
using arlcore::umaa::LargeListStatus;
using UMAA::MO::GlobalWaypointControl::GlobalWaypointType;

WaypointControlServiceProvider::WaypointControlServiceProvider(
    const arlcore::NumericGuid& source, std::shared_ptr<WaypointControlServiceProviderIo> io,
    IAutopilot* autopilot, double maxForwardSpeedMps, int maxListWaitCycles) :
    CommandProviderBase(source, io),
    sourceId_(source),
    autopilot_(autopilot),
    wpIo_(io),
    listReader_(io->listElementReader),
    maxForwardSpeedMps_(maxForwardSpeedMps),
    maxListWaitCycles_(maxListWaitCycles) {
  // A new waypoint route replaces an in-flight route (same driving resource).
  setBehavior(IncomingCommandBehavior::CANCEL_EXISTING);
}

void WaypointControlServiceProvider::resetPlanningState() {
  acquired_ = false;
  planned_ = false;
  listWaitCycles_ = 0;
  hasFailReason_ = false;
  pendingFailReason_ = CommandStatusReasonEnumType::SUCCEEDED;
}

void WaypointControlServiceProvider::relinquish(const std::weak_ptr<CmdSession> session) {
  autopilot_->clearSetpoint(DriveSource::WAYPOINT);
  autopilot_->arbiter().release(DriveSource::WAYPOINT);
  if (auto s = session.lock()) {
    listReader_.removeListByMetadata(s->getCommand().waypointsListMetadata());
  }
  resetPlanningState();
  sessionActive_ = false;
}

bool WaypointControlServiceProvider::validateWaypoints(
    const std::vector<GlobalWaypointType>& waypoints) const {
  if (waypoints.empty()) {
    return false;
  }
  for (const GlobalWaypointType& wp : waypoints) {
    const std::optional<SpeedValue> sp = tolerance::extractSpeed(wp.speed());
    if (sp.has_value() && maxForwardSpeedMps_ > 0.0 && sp->speedMps > maxForwardSpeedMps_) {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Waypoint speed " << sp->speedMps
        << " exceeds platform max forward speed " << maxForwardSpeedMps_)
      return false;
    }
  }
  return true;
}

bool WaypointControlServiceProvider::onCycle() {
  // Drain large-list element samples each cycle so the route can be assembled even when the
  // element samples arrive before (or after) the command metadata.
  listReader_.updateListElements();
  return true;
}

CommandStateResult WaypointControlServiceProvider::onCommanded(const std::weak_ptr<CmdSession> session) {
  auto s = session.lock();
  if (!s) {
    return CommandStateResult::ERROR;
  }

  const arlcore::NumericGuid sid = s->getSessionId();
  if (!sessionActive_ || sid != activeSession_) {
    resetPlanningState();
    activeSession_ = sid;
    sessionActive_ = true;
  }

  // Lost the resource to a higher-priority (vector) command while we were setting up.
  if (acquired_ && autopilot_->arbiter().wasRevoked(DriveSource::WAYPOINT)) {
    pendingFailReason_ = CommandStatusReasonEnumType::INTERRUPTED;
    hasFailReason_ = true;
    return CommandStateResult::ADVANCE;
  }

  // Acquire the (low-priority) driving resource. Denied if a vector command holds it.
  if (!acquired_) {
    if (autopilot_->arbiter().acquire(DriveSource::WAYPOINT)) {
      acquired_ = true;
    } else {
      pendingFailReason_ = CommandStatusReasonEnumType::RESOURCE_REJECTED;
      hasFailReason_ = true;
      return CommandStateResult::ADVANCE;  // fail fast in EXECUTING with the precise reason
    }
  }

  // Assemble the waypoint route from the large list, waiting until it is complete.
  if (!planned_) {
    const arlcore::umaa::LargeListResult<GlobalWaypointType> result =
        listReader_.getListFromMetadata(s->getCommand().waypointsListMetadata());
    if (result.status == LargeListStatus::VALID_LIST) {
      std::vector<GlobalWaypointType> waypoints;
      if (auto locked = result.list.lock()) {
        waypoints.assign(locked->begin(), locked->end());
      }
      if (!validateWaypoints(waypoints)) {
        pendingFailReason_ = CommandStatusReasonEnumType::VALIDATION_FAILED;
        hasFailReason_ = true;
        return CommandStateResult::ADVANCE;
      }
      if (!autopilot_->setWaypointSetpoint(waypoints)) {
        pendingFailReason_ = CommandStatusReasonEnumType::SERVICE_FAILED;
        hasFailReason_ = true;
        return CommandStateResult::ADVANCE;
      }
      planned_ = true;
      return CommandStateResult::ADVANCE;
    }

    // List not yet complete: stay in COMMANDED and retry, up to the wait budget.
    if (++listWaitCycles_ > maxListWaitCycles_) {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Timed out waiting for waypoint list to complete")
      pendingFailReason_ = CommandStatusReasonEnumType::SERVICE_FAILED;
      hasFailReason_ = true;
      return CommandStateResult::ADVANCE;
    }
    return CommandStateResult::OK;
  }

  return CommandStateResult::ADVANCE;
}

CommandStateResult WaypointControlServiceProvider::onExecuting(const std::weak_ptr<CmdSession> session) {
  // The brain drives off each navigation packet; keep the command executing. Failure (reject /
  // interrupt / objective failure) is surfaced via isCommandFailed.
  return CommandStateResult::OK;
}

bool WaypointControlServiceProvider::onUpdated(const std::weak_ptr<CmdSession> session,
    const GlobalWaypointCommandType& previousCmd, const GlobalWaypointCommandType& updatedCmd) {
  // Treat an update as a new route: replan from the (possibly updated) list next cycle.
  planned_ = false;
  listWaitCycles_ = 0;
  return true;
}

bool WaypointControlServiceProvider::isCommandCompleted(const std::weak_ptr<CmdSession> session) {
  return planned_ && autopilot_->waypointProgress().routeComplete;
}

CommandStatusReasonEnumType WaypointControlServiceProvider::isCommandFailed(
    const std::weak_ptr<CmdSession> session) {
  if (autopilot_->arbiter().wasRevoked(DriveSource::WAYPOINT)) {
    return CommandStatusReasonEnumType::INTERRUPTED;
  }
  if (hasFailReason_) {
    return pendingFailReason_;
  }
  if (planned_ && autopilot_->waypointProgress().failed) {
    return CommandStatusReasonEnumType::OBJECTIVE_FAILED;
  }
  return CommandStatusReasonEnumType::SUCCEEDED;
}

SendStatus WaypointControlServiceProvider::sendExecutionStatus(const GlobalWaypointCommandType& cmd) {
  if (!io_->cmdExeStatusSender.has_value()) {
    return SendStatus::SUCCESS;
  }
  const WaypointProgress prog = autopilot_->waypointProgress();
  GlobalWaypointExecutionStatusReportType report;
  report.sessionID() = cmd.sessionID();
  report.source().id() = sourceId_.getGuid();
  report.timeStamp() = arlcore::umaa::getTimestamp();
  report.positionAchieved() = prog.positionAchieved;
  if (prog.attitudeAchieved.has_value()) {
    report.attitudeAchieved() = prog.attitudeAchieved.value();
  }
  report.elevationAchieved() = prog.elevationAchieved;
  report.speedAchieved() = prog.speedAchieved;
  report.trackLineAchieved() = prog.trackLineAchieved;
  if (prog.crossTrackErrorM.has_value()) {
    report.crossTrackError() = prog.crossTrackErrorM.value();
  }
  report.distanceToWaypoint() = prog.distanceToWaypointM;
  report.distanceRemaining() = prog.distanceRemainingM;
  report.cumulativeDistance() = prog.cumulativeDistanceM;
  report.waypointsRemaining() = prog.waypointsRemaining;
  report.waypointID() = prog.waypointId.getGuid();
  // arrivalTime / timeToWaypoint estimation is left to a future iteration; stamp with now.
  report.arrivalTime() = arlcore::umaa::getTimestamp();
  report.timeToWaypoint() = arlcore::umaa::getTimestamp();
  return io_->cmdExeStatusSender.value()->send(report);
}

SendStatus WaypointControlServiceProvider::disposeExecutionStatus(const GlobalWaypointCommandType& cmd) {
  if (!io_->cmdExeStatusSender.has_value()) {
    return SendStatus::SUCCESS;
  }
  GlobalWaypointExecutionStatusReportType report;
  report.sessionID() = cmd.sessionID();
  report.source().id() = sourceId_.getGuid();
  report.timeStamp() = arlcore::umaa::getTimestamp();
  return io_->cmdExeStatusSender.value()->dispose(report);
}

bool WaypointControlServiceProvider::onCanceled(const std::weak_ptr<CmdSession> session) {
  relinquish(session);
  return true;
}

bool WaypointControlServiceProvider::onFailed(const std::weak_ptr<CmdSession> session) {
  relinquish(session);
  return true;
}

bool WaypointControlServiceProvider::onCompleted(const std::weak_ptr<CmdSession> session) {
  relinquish(session);
  return true;
}

}  // namespace arlcore::autopilot
