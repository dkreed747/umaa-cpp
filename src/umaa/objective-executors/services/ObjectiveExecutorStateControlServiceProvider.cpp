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

#include "ObjectiveExecutorStateControlServiceProvider.h"

namespace arlcore::umaa {

ObjectiveExecutorStateControlServiceProvider::ObjectiveExecutorStateControlServiceProvider(
  const NumericGuid& source,
  std::shared_ptr<ObjectiveExecutorStateControlServiceProviderIo> io,
  const std::function<std::optional<std::shared_ptr<ObjectiveBase>>()>& getActiveObjectiveCallback) :
  CommandProviderBase(source, io),
  getActiveObjectiveCallback_(getActiveObjectiveCallback) {}

bool ObjectiveExecutorStateControlServiceProvider::isCommandValid(const ObjectiveExecutorStateCommandType& cmd) {
  auto activeObjOpt = getActiveObjectiveCallback_();

  if (!activeObjOpt) {
    UMAA_LOG_WARN(util::SYSTEM_LOGGER, "No active objective in objective executor")
    return false;
  }

  if (auto activeObj = *activeObjOpt; activeObj->getId() != cmd.objectiveID()) {
    UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Received state command does not match active objective")
    return false;
  }

  UMAA_LOG_DEBUG(util::SYSTEM_LOGGER, "Objective referenced by state command exists! Command is valid.")
  return true;
}

CommandStateResult ObjectiveExecutorStateControlServiceProvider::onExecuting(const std::weak_ptr<CmdSession> session) {
  auto activeObjOpt = getActiveObjectiveCallback_();

  if (!activeObjOpt) {
    UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "No active objective in objective executor")
    return CommandStateResult::ERROR;
  }
  if (const std::shared_ptr<CmdSession> cmdSession = session.lock()) {
    std::shared_ptr<ObjectiveBase> activeObj = *activeObjOpt;
    UMAA_LOG_TRACE(util::SYSTEM_LOGGER, "Objective State " << activeObj->getObjectiveState())
    if (activeObj->getObjectiveState() == ObjectiveExecutorStateEnumType::PAUSING ||
        activeObj->getObjectiveState() == ObjectiveExecutorStateEnumType::RESUMING) {
      return CommandStateResult::OK;
    } else if (isCommandCompleted(session)) {
      return CommandStateResult::ADVANCE;
    }
    bool result = activeObj->commandObjectiveState(cmdSession->getCommand().objectiveState());
    return result ? CommandStateResult::OK : CommandStateResult::ERROR;
    // return activeObj->commandObjectiveState(cmdSession->getCommand().objectiveState());
  } else {
    UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Unable to acquire lock on command session")
    return CommandStateResult::ERROR;
  }
}

bool ObjectiveExecutorStateControlServiceProvider::isCommandCompleted(const std::weak_ptr<CmdSession> session) {
  auto activeObjOpt = getActiveObjectiveCallback_();

  if (!activeObjOpt) {
    UMAA_LOG_DEBUG(util::SYSTEM_LOGGER, "No active objective in objective executor")
    return false;
  }

  std::shared_ptr<ObjectiveBase> activeObj = *activeObjOpt;
  bool isCommandCompleted = false;

  if (std::shared_ptr<CmdSession> cmdSession = session.lock()) {
    switch (cmdSession->getCommand().objectiveState()) {
      case ObjectiveExecutorControlEnumType::EXECUTE:  // Intentional fall through
      case ObjectiveExecutorControlEnumType::RESUME:
        isCommandCompleted = activeObj->getObjectiveState() == ObjectiveExecutorStateEnumType::EXECUTING ||
                            activeObj->getObjectiveState() == ObjectiveExecutorStateEnumType::COMPLETED;
        break;
      case ObjectiveExecutorControlEnumType::PAUSE:
        isCommandCompleted = activeObj->getObjectiveState() == ObjectiveExecutorStateEnumType::PAUSED;
        break;
    }
  } else {
    UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Unable to acquire lock on command session")
    return false;
  }

  if (isCommandCompleted) {
    UMAA_LOG_DEBUG(util::SYSTEM_LOGGER, "Objective has reached desired state")
  } else {
    UMAA_LOG_TRACE(util::SYSTEM_LOGGER, "Objective has not yet reached desired state")
  }

  return isCommandCompleted;
}

CommandStatusReasonEnumType ObjectiveExecutorStateControlServiceProvider::isCommandFailed(
    const std::weak_ptr<CmdSession> session) {
  if (!getActiveObjectiveCallback_()) {
    return CommandStatusReasonEnumType::OBJECTIVE_FAILED;
  }

  return CommandStatusReasonEnumType::SUCCEEDED;
}

}  // namespace arlcore::umaa
