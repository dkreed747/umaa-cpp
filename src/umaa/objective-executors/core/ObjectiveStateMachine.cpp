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

#include <ObjectiveStateMachine.h>

#include <string>
#include <iostream>

namespace arlcore::umaa {

const std::map<const ObjectiveExecutorStateEnumType, const ObjectiveExecutorStateEnumType>
  ObjectiveStateMachine::SUCCESS_STATE_MAP =
    {{ObjectiveExecutorStateEnumType::QUEUED, ObjectiveExecutorStateEnumType::EXECUTING},
     {ObjectiveExecutorStateEnumType::EXECUTING, ObjectiveExecutorStateEnumType::COMPLETED},
     {ObjectiveExecutorStateEnumType::MODIFYING, ObjectiveExecutorStateEnumType::EXECUTING},
     {ObjectiveExecutorStateEnumType::PAUSING, ObjectiveExecutorStateEnumType::PAUSED},
     {ObjectiveExecutorStateEnumType::RESUMING, ObjectiveExecutorStateEnumType::EXECUTING},
     {ObjectiveExecutorStateEnumType::CANCELING, ObjectiveExecutorStateEnumType::CANCELED}};

bool ObjectiveStateMachine::advanceState() {
  auto next = SUCCESS_STATE_MAP.find(currentState_);
  if (next == SUCCESS_STATE_MAP.end()) {
    return false;
  }
  currentState_ = next->second;
  currentReason_ = ObjectiveExecutorStateReasonEnumType::SUCCEEDED;
  return true;
}

bool ObjectiveStateMachine::update() {
  if (currentState_ == ObjectiveExecutorStateEnumType::EXECUTING) {
    currentState_ = ObjectiveExecutorStateEnumType::MODIFYING;
    currentReason_ = ObjectiveExecutorStateReasonEnumType::BUS_MSG_UPDATE;
    return true;
  }
  return false;
}

bool ObjectiveStateMachine::pause() {
  if (currentState_ == ObjectiveExecutorStateEnumType::EXECUTING) {
    currentState_ = ObjectiveExecutorStateEnumType::PAUSING;
    currentReason_ = ObjectiveExecutorStateReasonEnumType::COMMANDED;
    return true;
  }
  return false;
}

bool ObjectiveStateMachine::resume() {
  if (currentState_ == ObjectiveExecutorStateEnumType::PAUSED) {
    currentState_ = ObjectiveExecutorStateEnumType::RESUMING;
    currentReason_ = ObjectiveExecutorStateReasonEnumType::COMMANDED;
    return true;
  }
  return false;
}

bool ObjectiveStateMachine::cancel() {
  if (!isStateFinal(currentState_) && currentState_ != ObjectiveExecutorStateEnumType::CANCELING) {
    currentState_ = ObjectiveExecutorStateEnumType::CANCELING;
    currentReason_ = ObjectiveExecutorStateReasonEnumType::BUS_MSG_DISPOSE;
    return true;
  }
  return false;
}

bool ObjectiveStateMachine::fail(ObjectiveExecutorStateReasonEnumType reason) {
  if (!isStateFinal(currentState_) && currentState_ != ObjectiveExecutorStateEnumType::CANCELING) {
    bool isValid = false;
    switch (reason) {
      // These reasons are valid from any non-final state (besides CANCELING)
      case ObjectiveExecutorStateReasonEnumType::CANNOT_PERFORM_UNDER_CONSTRAINTS:
      case ObjectiveExecutorStateReasonEnumType::INTERNAL_FAILURE:
      case ObjectiveExecutorStateReasonEnumType::LOWER_SERVICE_FAILED:
      case ObjectiveExecutorStateReasonEnumType::LOWER_SERVICE_TIMEOUT:
      case ObjectiveExecutorStateReasonEnumType::LOWER_SERVICE_REJECTED:
      case ObjectiveExecutorStateReasonEnumType::LOWER_SERVICE_INTERRUPTED:
      case ObjectiveExecutorStateReasonEnumType::OBJECTIVE_REPLACED:
      case ObjectiveExecutorStateReasonEnumType::COMMAND_VALIDATION_FAILED:
        isValid = true;
        break;
      default:
        break;
    }

    if (isValid) {
      currentState_ = ObjectiveExecutorStateEnumType::FAILED;
      currentReason_ = reason;
      return true;
    }
  }
  return false;
}

void ObjectiveStateMachine::reset() {
  currentState_ = ObjectiveExecutorStateEnumType::QUEUED;
  currentReason_ = ObjectiveExecutorStateReasonEnumType::SUCCEEDED;
}

}  // namespace arlcore::umaa
