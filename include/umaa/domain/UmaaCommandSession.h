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

#ifndef INCLUDE_UMAA_DOMAIN_UMAACOMMANDSESSION_H_
#define INCLUDE_UMAA_DOMAIN_UMAACOMMANDSESSION_H_

#include <map>
#include <array>
#include <string>

#include "EnumTypes.h"
#include "GUID.h"

namespace arl {

class UmaaCommandSession {
 public:
  UmaaCommandSession(NumericGUID_t source, NumericGUID_t session_id);
  UmaaCommandSession() = default;
  virtual ~UmaaCommandSession() = default;

  bool handleSuccess(const std::string& message = "");
  bool handleUpdate(const std::string& message = "");
  bool handleCancel(const std::string& message = "");
  bool handleFailure(CommandStatus reason, const std::string& message = "");

  CommandState getState() const { return current_state_; }
  CommandStatus getStatus() const { return status_; }

  NumericGUID_t getSource() const { return source_; }
  NumericGUID_t getSessionId() const { return session_id_; }


 protected:
  void setSource(NumericGUID_t src) { source_ = src; }
  void setSessionId(NumericGUID_t id) { session_id_ = id; }
  void setCurrentState(CommandState state) { current_state_ = state; }
  void setStatus(CommandStatus status) { status_ = status; }

  virtual void onStateChange(CommandState prev, CommandState next,
      CommandStatus reason, const std::string& message) = 0;

 private:
  static const std::map<const CommandState, const CommandState> SUCCESS_STATE_MAP;

  NumericGUID_t source_;
  NumericGUID_t session_id_;
  CommandState current_state_ = CommandState::INITIAL_STATE;  // Command Status
  CommandStatus status_ = CommandStatus::SUCCEEDED;  // Command Status Reason


  inline bool isFinalState(CommandState s) const {
    return !(s == CommandState::ISSUED || s == CommandState::COMMANDED || s == CommandState::EXECUTING);
  }

  /**
   * Transitions
   * INITIAL_STATE -> ISSUED : SUCCEEDED
   * ISSUED -> ISSUED : UPDATED
   * ISSUED -> COMMANDED : SUCCEEDED
   * ISSUED -> FAILED : VALIDATION_FAILED, RESOURCE_FAILED, INTERRUPTED, TIMEOUT, SERVICE_FAILED
   * ISSUED -> CANCELED : CANCELED
   * COMMANDED -> ISSUED : UPDATED
   * COMMANDED -> EXECUTING : SUCCEEDED
   * COMMANDED -> FAILED : RESOURCE_REJECTED, INTERRUPTED, TIMEOUT, SERVICE_FAILED
   * COMMANDED -> CANCELED : CANCELED
   * EXECUTING -> ISSUED : UPDATED
   * EXECUTING -> COMPLETED : SUCCEEDED
   * EXECUTING -> FAILED : OBJECTIVE_FAILED, RESOURCE_FAILED, INTERRUPTED, TIMEOUT, SERVICE_FAILED
   * EXECUTING -> CANCELED : CANCELED
   */
};

}  // namespace arl

#endif  // INCLUDE_UMAA_DOMAIN_UMAACOMMANDSESSION_H_
