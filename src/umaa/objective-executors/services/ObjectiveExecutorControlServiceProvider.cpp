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

#include "ObjectiveExecutorControlServiceProvider.h"

namespace arlcore::umaa {

std::optional<std::shared_ptr<ObjectiveBase>> ObjectiveExecutorControlServiceProvider::activeObjective_ = std::nullopt;

ObjectiveExecutorControlServiceProvider::ObjectiveExecutorControlServiceProvider(
  const NumericGuid &source, std::shared_ptr<ObjectiveExecutorControlServiceProviderIo> io,
  std::shared_ptr<ObjectiveFactory> objFactory) :
  CommandProviderBase(source, io),
  sourceId_(source),
  objFactory_(move(objFactory)) {
    setBehavior(arlcore::umaa::services::IncomingCommandBehavior::REJECT_INCOMING);
}

ObjectiveExecutorControlServiceProvider::~ObjectiveExecutorControlServiceProvider() {
  unregisterActiveObjective();
  activeObjective_.reset();
}

bool ObjectiveExecutorControlServiceProvider::onCycle() {
  // If provider is IDLE reset active objective data
  if (std::optional<CommandStatusEnumType> status = getCommandStatus();
      !status.has_value() || CommandStateMachine::isStateFinal(status.value())) {
    unregisterActiveObjective();
    activeObjective_.reset();
  }

  return true;
}

bool ObjectiveExecutorControlServiceProvider::onCanceled(const std::weak_ptr<CmdSession> session) {
  return activeObjective_ ? activeObjective_.value()->transitionObjectiveStateToCanceled() : true;
}

bool ObjectiveExecutorControlServiceProvider::onUpdated(const std::weak_ptr<CmdSession> session,
    const ObjectiveExecutorCommandType& previousCmd,
    const ObjectiveExecutorCommandType& updatedCmd) {
  return activeObjective_ ? activeObjective_.value()->transitionObjectiveStateToModifying() : true;
}

bool ObjectiveExecutorControlServiceProvider::isCommandValid(const ObjectiveExecutorCommandType& cmd) {
  std::optional<std::shared_ptr<arlcore::umaa::ObjectiveBase>> objOpt = objFactory_->build(cmd.objective());

  if (!objOpt) {
    UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Failed to create valid objective from command data")
    return false;
  }

  if (constraintProvider_.has_value()) {
    constraintProvider_->get()->registerObserver(objOpt.value());
  }

  if (!objOpt.value()->isObjectiveValid()) {
    UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Objective specific validation logic failed")
    return false;
  }

  activeObjective_ = objOpt;

  UMAA_LOG_DEBUG(util::SYSTEM_LOGGER, "Received ObjectiveExecutorControlCommand is valid")
  return true;
}

bool ObjectiveExecutorControlServiceProvider::isCommandCompleted(const std::weak_ptr<CmdSession> session) {
  return activeObjective_.value()->getObjectiveState() == ObjectiveExecutorStateEnumType::COMPLETED;
}

CommandStatusReasonEnumType ObjectiveExecutorControlServiceProvider::isCommandFailed(
    const std::weak_ptr<CmdSession> session) {
  return activeObjective_.value()->getObjectiveState() == ObjectiveExecutorStateEnumType::FAILED ?
    CommandStatusReasonEnumType::OBJECTIVE_FAILED : CommandStatusReasonEnumType::SUCCEEDED;
}

SendStatus ObjectiveExecutorControlServiceProvider::sendExecutionStatus(const ObjectiveExecutorCommandType& cmd) {
  // Guard clause for sending execution status after the objective has been removed
  if (!activeObjective_ || !io_->cmdExeStatusSender.has_value()) {
    return SendStatus::SUCCESS;
  }

  ObjectiveExecutorExecutionStatusReportType exeStatReport;
  exeStatReport.missionID() = cmd.missionID();
  exeStatReport.taskID() = cmd.taskID();
  exeStatReport.sessionID() = cmd.sessionID();
  exeStatReport.source().id() = sourceId_.getGuid();
  exeStatReport.objectiveDetailedStatus().objectiveID() = activeObjective_.value()->getId().getGuid();
  exeStatReport.objectiveDetailedStatus().objectiveStatus() = activeObjective_.value()->getObjectiveState();
  exeStatReport.objectiveDetailedStatus().objectiveStatusReason() = activeObjective_.value()->getObjectiveStateReason();
  exeStatReport.objectiveDetailedStatus().feedback() = "Waypoints are executing.";
  return io_->cmdExeStatusSender.value()->send(exeStatReport);
}

SendStatus ObjectiveExecutorControlServiceProvider::disposeExecutionStatus(const ObjectiveExecutorCommandType& cmd) {
  if (!io_->cmdExeStatusSender.has_value()) {
    return SendStatus::SUCCESS;
  }
  ObjectiveExecutorExecutionStatusReportType disposeReport;
  disposeReport.missionID() = cmd.missionID();
  disposeReport.taskID() = cmd.taskID();
  disposeReport.sessionID() = cmd.sessionID();
  disposeReport.source().id() = sourceId_.getGuid();

  return io_->cmdExeStatusSender.value()->dispose(disposeReport);
}

void ObjectiveExecutorControlServiceProvider::setActiveConstraintsProvider(
    std::optional<std::shared_ptr<conditional::ActiveConstraintsControlProvider>> constraintProvider) {
  constraintProvider_ = constraintProvider;
}

std::optional<std::shared_ptr<ObjectiveBase>>
  ObjectiveExecutorControlServiceProvider::getActiveObjective() {
    return activeObjective_;
}

}  // namespace arlcore::umaa
