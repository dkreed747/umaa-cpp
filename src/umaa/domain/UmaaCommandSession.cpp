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

#include <UmaaCommandSession.h>
#include <string>
#include <iostream>

namespace arl {

  const std::map<const CommandState, const CommandState> UmaaCommandSession::SUCCESS_STATE_MAP = {
      {CommandState::INITIAL_STATE, CommandState::ISSUED},
      {CommandState::ISSUED, CommandState::COMMANDED},
      {CommandState::COMMANDED, CommandState::EXECUTING},
      {CommandState::EXECUTING, CommandState::COMPLETED},
      {CommandState::COMPLETED, CommandState::INVALID},
      {CommandState::COMPLETED, CommandState::INVALID},
      {CommandState::CANCELED, CommandState::INVALID},
      {CommandState::FAILED, CommandState::INVALID},
      {CommandState::INVALID, CommandState::INVALID}
  };

  UmaaCommandSession::UmaaCommandSession(NumericGUID_t source, NumericGUID_t session_id) :
      source_(source), session_id_(session_id) {
  }

  bool UmaaCommandSession::handleSuccess(const std::string& message) {
    CommandState next = SUCCESS_STATE_MAP.at(current_state_);

    if (next != CommandState::INVALID) {
      CommandState prev = current_state_;
      current_state_ = next;
      status_ = CommandStatus::SUCCEEDED;
      onStateChange(prev, current_state_, CommandStatus::SUCCEEDED, message);

      return true;
    }
    return false;
  }

  bool UmaaCommandSession::handleUpdate(const std::string& message) {
    if (!isFinalState(current_state_) && current_state_ != CommandState::INITIAL_STATE) {
      CommandState prev = current_state_;
      current_state_ = CommandState::ISSUED;
      status_ = CommandStatus::UPDATED;
      onStateChange(prev, current_state_, status_, message);

      return true;
    }

    return false;
  }

  bool UmaaCommandSession::handleCancel(const std::string& message) {
    if (!isFinalState(current_state_) && current_state_ != CommandState::INITIAL_STATE) {
      CommandState prev = current_state_;
      current_state_ = CommandState::CANCELED;
      status_ = CommandStatus::CANCELED;
      onStateChange(prev, current_state_, status_, message);

      return true;
    }
    return false;
  }

  bool UmaaCommandSession::handleFailure(CommandStatus reason, const std::string& message) {
    CommandState prev = current_state_;

    if (!isFinalState(current_state_) && current_state_ != CommandState::INITIAL_STATE) {
      bool isValid = false;
      switch (reason) {
      // These reasons are valid from any non-final state
      case CommandStatus::SERVICE_FAILED:
      case CommandStatus::INTERRUPTED:
      case CommandStatus::TIMEOUT:
        isValid = true;
        break;
      case CommandStatus::RESOURCE_FAILED:
        isValid = (current_state_ == CommandState::ISSUED || current_state_ == CommandState::EXECUTING);
        break;
      case CommandStatus::VALIDATION_FAILED:
        isValid = (current_state_ == CommandState::ISSUED);
        break;
      case CommandStatus::RESOURCE_REJECTED:
        isValid = (current_state_ == CommandState::COMMANDED);
        break;
      case CommandStatus::OBJECTIVE_FAILED:
        isValid = (current_state_ == CommandState::EXECUTING);
        break;
      default:
        break;
      }

      if (isValid) {
        current_state_ = CommandState::FAILED;
        status_ = reason;
        onStateChange(prev, current_state_, status_, message);
        return true;
      }
    }
    return false;
  }

}  // namespace arl
