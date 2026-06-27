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

#ifndef INCLUDE_UMAA_COMMANDSTATEMACHINE_H_
#define INCLUDE_UMAA_COMMANDSTATEMACHINE_H_

#include <map>
#include <array>
#include <string>

#include "UMAA/Common/MaritimeEnumeration/MaritimeEnumerationSets.hpp"

using CommandStatusEnumType = UMAA::Common::MaritimeEnumeration::CommandStatusEnumModule::CommandStatusEnumType;
using CommandStatusReasonEnumType =
    UMAA::Common::MaritimeEnumeration::CommandStatusReasonEnumModule::CommandStatusReasonEnumType;

namespace arlcore::umaa {

/**
 * Valid Transitions
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

//! \brief A state machine that manages the state of a UMAA command session that has already been issued.
class CommandStateMachine {
 public:
  CommandStateMachine() = default;
  virtual ~CommandStateMachine() = default;

  //! \brief Advance the state machine to the next state if it exists
  //! \return whether the state could be advanced
  bool advanceState();

  //! \brief Change the current state to ISSUED if the transition is valid
  //! \return whether the state could be updated to ISSUED
  bool update();

  //! \brief Change the current state to CANCELED if the transition is valid
  //! \return whether the state could be updated to CANCELED
  bool cancel();

  //! \brief Change the current state to FAILED if the transition is valid for the specified reason
  //! \param reason The reason given for the failure
  //! \return whether the statue could be updated to FAILED
  bool fail(CommandStatusReasonEnumType reason);

  //! \brief Resets the state machine to its initial state
  void reset();

  //! \brief Gets the current state of the state machine
  //! \return the current state
  CommandStatusEnumType getState() const { return currentState_; }

  //! \brief Gets the current state change reason of the state machine
  //! \return the current status reason
  CommandStatusReasonEnumType getReason() const { return currentReason_; }

  //! \brief Check if the current state of the state machine is final (i.e. can no longer be updated)
  //! \return whether the current state is considered final
  bool isFinal() const { return isStateFinal(currentState_); }

  //! \brief Helper function to check whether a state is considered final
  //! \param state The state to check
  //! \return whether the provided state is considered final
  static bool isStateFinal(CommandStatusEnumType state) {
    return SUCCESS_STATE_MAP_.count(state) == 0;
  }

 private:
  static const std::map<const CommandStatusEnumType, const CommandStatusEnumType> SUCCESS_STATE_MAP_;
  CommandStatusEnumType currentState_ = CommandStatusEnumType::ISSUED;
  CommandStatusReasonEnumType currentReason_ = CommandStatusReasonEnumType::SUCCEEDED;
};

}  // namespace arlcore::umaa

#endif  // INCLUDE_UMAA_COMMANDSTATEMACHINE_H_
