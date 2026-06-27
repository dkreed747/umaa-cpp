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

#ifndef INCLUDE_UMAA_OBJECTIVE_EXECUTORS_CORE_OBJECTIVESTATEMACHINE_H_
#define INCLUDE_UMAA_OBJECTIVE_EXECUTORS_CORE_OBJECTIVESTATEMACHINE_H_

#include <map>
#include <array>
#include <string>

#include "UMAA/Common/MaritimeEnumeration/MaritimeEnumerationSets.hpp"

namespace arlcore::umaa {

using UMAA::Common::MaritimeEnumeration::ObjectiveExecutorStateEnumModule::ObjectiveExecutorStateEnumType;
using UMAA::Common::MaritimeEnumeration::ObjectiveExecutorStateReasonEnumModule::ObjectiveExecutorStateReasonEnumType;

/**
 * Valid Transitions
 * QUEUED -> EXECUTING : COMMANDED
 * QUEUED -> CANCELING : BUS_MSG_DISPOSE
 * QUEUED -> FAILED : CANNOT_PERFORM_UNDER_CONSTRAINTS, INTERNAL_FAILURE, LOWER_SERVICE_FAILED, LOWER_SERVICE_TIMEOUT,
 *                    LOWER_SERVICE_REJECTED, LOWER_SERVICE_INTERRUPTED, OBJECTIVE_REPLACED, COMMAND_VALIDATION_FAILED
 * CANCELING -> CANCELED : SUCCEEDED
 * EXECUTING -> COMPLETE : SUCCEEDED
 * EXECUTING -> PAUSING : COMMANDED
 * EXECUTING -> MODIFYING : BUS_MSG_UPDATE
 * EXECUTING -> CANCELING : BUS_MSG_DISPOSE
 * EXECUTING -> FAILED : CANNOT_PERFORM_UNDER_CONSTRAINTS, INTERNAL_FAILURE, LOWER_SERVICE_FAILED,
 *                       LOWER_SERVICE_TIMEOUT, LOWER_SERVICE_REJECTED, LOWER_SERVICE_INTERRUPTED, OBJECTIVE_REPLACED,
 *                       COMMAND_VALIDATION_FAILED
 * MODIFYING -> EXECUTING : SUCCEEDED
 * MODIFYING -> CANCELING : BUS_MSG_DISPOSE
 * MODIFYING -> FAILED : CANNOT_PERFORM_UNDER_CONSTRAINTS, INTERNAL_FAILURE, LOWER_SERVICE_FAILED,
 *                       LOWER_SERVICE_TIMEOUT, LOWER_SERVICE_REJECTED, LOWER_SERVICE_INTERRUPTED, OBJECTIVE_REPLACED,
 *                       COMMAND_VALIDATION_FAILED
 * PAUSING -> PAUSED : SUCCEEDED
 * PAUSING -> CANCELING : BUS_MSG_DISPOSE
 * PAUSING -> FAILED : CANNOT_PERFORM_UNDER_CONSTRAINTS, INTERNAL_FAILURE, LOWER_SERVICE_FAILED,
 *                       LOWER_SERVICE_TIMEOUT, LOWER_SERVICE_REJECTED, LOWER_SERVICE_INTERRUPTED, OBJECTIVE_REPLACED,
 *                       COMMAND_VALIDATION_FAILED
 * PAUSED -> RESUMING : COMMANDED
 * PAUSED -> CANCELING : BUS_MSG_DISPOSE
 * PAUSED -> FAILED : CANNOT_PERFORM_UNDER_CONSTRAINTS, INTERNAL_FAILURE, LOWER_SERVICE_FAILED,
 *                       LOWER_SERVICE_TIMEOUT, LOWER_SERVICE_REJECTED, LOWER_SERVICE_INTERRUPTED, OBJECTIVE_REPLACED,
 *                       COMMAND_VALIDATION_FAILED
 * RESUMING -> EXECUTING : SUCCEEDED
 * RESUMING -> CANCELING : BUS_MSG_DISPOSE
 * RESUMING -> FAILED : CANNOT_PERFORM_UNDER_CONSTRAINTS, INTERNAL_FAILURE, LOWER_SERVICE_FAILED,
 *                       LOWER_SERVICE_TIMEOUT, LOWER_SERVICE_REJECTED, LOWER_SERVICE_INTERRUPTED, OBJECTIVE_REPLACED,
 *                       COMMAND_VALIDATION_FAILED
 */

//! \brief A state machine that manages the state of a UMAA command session that has already been issued.
class ObjectiveStateMachine {
 public:
  ObjectiveStateMachine() = default;
  virtual ~ObjectiveStateMachine() = default;

  //! \brief Advance the state machine to the next state if it exists
  //! \return whether the state could be advanced
  bool advanceState();

  //! \brief Change the current state to ISSUED if the transition is valid
  //! \return whether the state could be updated to ISSUED
  bool update();

  //! \brief Change the current state to PAUSING if the transition is valid
  //! \return whether the state could be updated to PAUSING
  bool pause();

  //! \brief Change the current state to RESUMING if the transition is valid
  //! \return whether the state could be updated to RESUMING
  bool resume();

  //! \brief Change the current state to CANCELING if the transition is valid
  //! \return whether the state could be updated to CANCELING
  bool cancel();

  //! \brief Change the current state to FAILED if the transition is valid for the specified reason
  //! \param reason The reason given for the failure
  //! \return whether the state could be updated to FAILED
  bool fail(ObjectiveExecutorStateReasonEnumType reason);

  //! \brief Resets the state machine to its initial state
  void reset();

  //! \brief Gets the current state of the state machine
  //! \return the current state
  ObjectiveExecutorStateEnumType getState() const { return currentState_; }

  //! \brief Gets the current state change reason of the state machine
  //! \return the current status reason
  ObjectiveExecutorStateReasonEnumType getReason() const { return currentReason_; }

  //! \brief Check if the current state of the state machine is final (i.e. can no longer be updated)
  //! \return whether the current state is considered final
  bool isFinal() const { return isStateFinal(currentState_); }

  //! \brief Helper function to check whether a state is considered final
  //! @param state The state to check
  //! @return whether the provided state is considered final
  static bool isStateFinal(ObjectiveExecutorStateEnumType state) {
    switch (state) {
      case ObjectiveExecutorStateEnumType::CANCELED:
      case ObjectiveExecutorStateEnumType::COMPLETED:
      case ObjectiveExecutorStateEnumType::FAILED:
        return true;
      default:
        return false;
    }
  }

 private:
  static const std::map<const ObjectiveExecutorStateEnumType, const ObjectiveExecutorStateEnumType> SUCCESS_STATE_MAP;
  ObjectiveExecutorStateEnumType currentState_ = ObjectiveExecutorStateEnumType::QUEUED;
  ObjectiveExecutorStateReasonEnumType currentReason_ = ObjectiveExecutorStateReasonEnumType::SUCCEEDED;
};

}  // namespace arlcore::umaa

#endif  // INCLUDE_UMAA_OBJECTIVE_EXECUTORS_CORE_OBJECTIVESTATEMACHINE_H_
