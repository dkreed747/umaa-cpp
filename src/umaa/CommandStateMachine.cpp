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

#include <CommandStateMachine.h>

#include <string>
#include <iostream>

namespace arlcore::umaa {

  const std::map<const CommandStatusEnumType, const CommandStatusEnumType> CommandStateMachine::SUCCESS_STATE_MAP_ = {
      {CommandStatusEnumType::ISSUED, CommandStatusEnumType::COMMANDED},
      {CommandStatusEnumType::COMMANDED, CommandStatusEnumType::EXECUTING},
      {CommandStatusEnumType::EXECUTING, CommandStatusEnumType::COMPLETED}
  };

  bool CommandStateMachine::advanceState() {
    auto next = SUCCESS_STATE_MAP_.find(currentState_);
    if (next == SUCCESS_STATE_MAP_.end()) {
      return false;
    }
    currentState_ = next->second;
    currentReason_ = CommandStatusReasonEnumType::SUCCEEDED;
    return true;
  }

  bool CommandStateMachine::update() {
    if (!isStateFinal(currentState_)) {
      currentState_ = CommandStatusEnumType::ISSUED;
      currentReason_ = CommandStatusReasonEnumType::UPDATED;
      return true;
    }
    return false;
  }

  bool CommandStateMachine::cancel() {
    if (!isStateFinal(currentState_)) {
      currentState_ = CommandStatusEnumType::CANCELED;
      currentReason_ = CommandStatusReasonEnumType::CANCELED;
      return true;
    }
    return false;
  }

  bool CommandStateMachine::fail(CommandStatusReasonEnumType reason) {
    if (!isStateFinal(currentState_)) {
      bool isValid = false;
      switch (reason) {
      // These reasons are valid from any non-final state
      case CommandStatusReasonEnumType::SERVICE_FAILED:
      case CommandStatusReasonEnumType::INTERRUPTED:
      case CommandStatusReasonEnumType::TIMEOUT:
        isValid = true;
        break;
      case CommandStatusReasonEnumType::RESOURCE_FAILED:
        isValid = (currentState_ == CommandStatusEnumType::ISSUED || currentState_ == CommandStatusEnumType::EXECUTING);
        break;
      case CommandStatusReasonEnumType::VALIDATION_FAILED:
        isValid = (currentState_ == CommandStatusEnumType::ISSUED);
        break;
      case CommandStatusReasonEnumType::RESOURCE_REJECTED:
        isValid = (currentState_ == CommandStatusEnumType::COMMANDED);
        break;
      case CommandStatusReasonEnumType::OBJECTIVE_FAILED:
        isValid = (currentState_ == CommandStatusEnumType::EXECUTING);
        break;
      default:
        break;
      }

      if (isValid) {
        currentState_ = CommandStatusEnumType::FAILED;
        currentReason_ = reason;
        return true;
      }
    }
    return false;
  }

  void CommandStateMachine::reset() {
    currentState_ = CommandStatusEnumType::ISSUED;
    currentReason_ = CommandStatusReasonEnumType::SUCCEEDED;
  }

}  // namespace arlcore::umaa
