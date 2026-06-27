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

#include <TaskStateMachine.h>

#include <string>
#include <iostream>
#include "Logger.h"

namespace arlcore::umaa {

  const std::map<const TaskStateEnumType, const TaskStateEnumType> TaskStateMachine::SUCCESS_STATE_MAP_ = {
    {TaskStateEnumType::NOT_PLANNED, TaskStateEnumType::PLANNING},
    {TaskStateEnumType::PLANNING, TaskStateEnumType::PLANNED},
    {TaskStateEnumType::PLANNED, TaskStateEnumType::AWAITING_EXECUTION_APPROVAL},
    {TaskStateEnumType::AWAITING_EXECUTION_APPROVAL, TaskStateEnumType::EXECUTION_APPROVED},
    {TaskStateEnumType::EXECUTION_APPROVED, TaskStateEnumType::NOT_QUEUED},
    {TaskStateEnumType::NOT_QUEUED, TaskStateEnumType::QUEUING},
    {TaskStateEnumType::QUEUING, TaskStateEnumType::QUEUED},
    {TaskStateEnumType::QUEUED, TaskStateEnumType::EXECUTING},
    {TaskStateEnumType::EXECUTING, TaskStateEnumType::COMPLETED},
    {TaskStateEnumType::PAUSING, TaskStateEnumType::PAUSED},
    {TaskStateEnumType::RESUMING, TaskStateEnumType::NOT_PLANNED},
    {TaskStateEnumType::CANCELING, TaskStateEnumType::CANCELED},
    {TaskStateEnumType::RESTARTING, TaskStateEnumType::NOT_PLANNED}
  };

  bool TaskStateMachine::advanceState() {
    auto next = SUCCESS_STATE_MAP_.find(currentState_);
    if (next == SUCCESS_STATE_MAP_.end()) {
      return false;
    }
    currentState_ = next->second;
    return true;
  }

  bool TaskStateMachine::update() {
    switch (currentState_) {
      case TaskStateEnumType::PLANNING:
      case TaskStateEnumType::PLANNED:
      case TaskStateEnumType::AWAITING_EXECUTION_APPROVAL:
      case TaskStateEnumType::EXECUTION_APPROVED:
      case TaskStateEnumType::NOT_QUEUED:
      case TaskStateEnumType::QUEUING:
      case TaskStateEnumType::QUEUED:
      case TaskStateEnumType::EXECUTING:
        currentState_ = TaskStateEnumType::NOT_PLANNED;
        return true;
      default:
        UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Unable to update from current state: " << currentState_)
    }
    return false;
  }

  bool TaskStateMachine::pause() {
    switch (currentState_) {
      case TaskStateEnumType::NOT_PLANNED:
      case TaskStateEnumType::PLANNING:
      case TaskStateEnumType::PLANNED:
      case TaskStateEnumType::AWAITING_EXECUTION_APPROVAL:
      case TaskStateEnumType::EXECUTION_APPROVED:
      case TaskStateEnumType::NOT_QUEUED:
      case TaskStateEnumType::QUEUING:
      case TaskStateEnumType::QUEUED:
      case TaskStateEnumType::EXECUTING:
        currentState_ = TaskStateEnumType::PAUSING;
        return true;
      default:
        UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Unable to pause from current state: " << currentState_)
    }
    return false;
  }

  bool TaskStateMachine::resume() {
    if (currentState_ == TaskStateEnumType::PAUSED) {
      currentState_ = TaskStateEnumType::RESUMING;
      return true;
    }
    UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Unable to resume, not currently paused.")
    return false;
  }

  bool TaskStateMachine::cancel() {
    if (!isStateFinal(currentState_) && currentState_ != TaskStateEnumType::CANCELING) {
      currentState_ = TaskStateEnumType::CANCELING;
      return true;
    }
    UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Unable to cancel from current state: " << currentState_)
    return false;
  }

  bool TaskStateMachine::fail() {
    if (!isStateFinal(currentState_) && currentState_ != TaskStateEnumType::CANCELING) {
      currentState_ = TaskStateEnumType::FAILED;
      return true;
    }
    UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Unable to fail from current state: " << currentState_)
    return false;
  }

  bool TaskStateMachine::restart() {
    switch (currentState_) {
      case TaskStateEnumType::NOT_PLANNED:
        UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Unable to restart from current state: " << currentState_)
        return false;
      default:
        currentState_ = TaskStateEnumType::RESTARTING;
    }
    return true;
  }

void TaskStateMachine::reset() {
  currentState_ = TaskStateEnumType::NOT_PLANNED;
}

}  // namespace arlcore::umaa
