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

#ifndef INCLUDE_UMAA_TASKSTATEMACHINE_H_
#define INCLUDE_UMAA_TASKSTATEMACHINE_H_

#include <map>
#include <array>
#include <string>

#include "UMAA/Common/MaritimeEnumeration/MaritimeEnumerationSets.hpp"

using TaskStateEnumType = UMAA::Common::MaritimeEnumeration::TaskStateEnumModule::TaskStateEnumType;

namespace arlcore::umaa {

/**
 * Task States
 * AWAITING_EXECUTION_APPROVAL, // The mission plan, mission task, or mission objective is awaiting execution approval.
 * CANCELED, // The mission plan, mission task, or mission objective has been cancelled.
 * CANCELING, // The mission plan, mission task, or mission objective is in the process of being cancelled.
 * COMPLETED, // The mission plan, mission task, or mission objective been completed. Collection tasks are considered complete when the resulting product is processed and disseminated. All other tasks are complete once the vehicle transitions from the executing state (vehicle releases weapon, stops jamming, etc.).
 * EXECUTING, // The mission plan, mission task, or mission objective has begun execution (slews sensor and begins collect, begins to prepare weapons for release, starts jamming, etc.). This state defines the point of no return for a mission plan, mission task, or mission objective. Once transitioning to this state, the mission plan, mission task, or mission objective can no longer be reallocated to another UxS or vehicle unless it transitions to the FAILED state.
 * EXECUTION_APPROVED, // The mission plan, mission task, or mission objective been approved for execution.
 * FAILED, // The mission plan, mission task, or mission objective has failed. The UxS node has determined that no vehicles within the UxS can achieve the mission plan, mission task, or mission objective.
 * NOT_PLANNED, // The mission plan, mission task, or mission objective has not been planned.
 * NOT_QUEUED, // The mission plan, mission task, or mission objective has not been queued for execution.
 * PAUSED, // Used to pause the execution of an approved mission plan, approved mission task, or approved mission objective.
 * PAUSING, // The mission plan, mission task, or mission objective is in the process of being paused.
 * PLANNED, // The mission plan, mission task, or mission objective has been planned, indicating that it is part of an approved and active detailed mission plan.
 * PLANNING, // The mission plan, mission task, or mission objective is still in the planning state.
 * QUEUED, // The mission plan, mission task, or mission objective has been queued for execution.
 * QUEUING, // The mission plan, mission task, or mission objective is being queued (e.g., uploading to vehicle) for execution.
 * RESTARTING, // The mission plan, mission task, or mission objective is in the process of being restarted.
 * RESUMING // The mission plan, mission task, or mission objective is in the process of being resumed.
 */

/**
 * Valid Transitions
 * NOT_PLANNED -> PLANNING, PAUSING :
 * PLANNING -> PLANNED, PAUSING, RESTARTING :
 * PLANNED -> AWAITING_EXECUTION_APPROVAL, EXECUTION_APPROVED, PAUSING, RESTARTING :
 * AWAITING_EXECUTION_APPROVAL -> EXECUTION_APPROVED, CANCELING, PAUSING, RESTARTING :
 * EXECUTION_APPROVED -> NOT_QUEUED, PAUSING, RESTARTING :
 * NOT_QUEUED -> QUEUING, PAUSING, RESTARTING :
 * QUEUING -> QUEUED, PAUSING, RESTARTING :
 * QUEUED -> EXECUTING, PAUSING, RESTARTING :
 * EXECUTING -> COMPLETED, FAILED, PAUSING, RESTARTING :
 * CANCELING -> CANCELED, RESTARTING :
 * PAUSING -> PAUSED, RESTARTING :
 * PAUSED -> RESUMING, RESTARTING :
 * RESUMING -> NOT_PLANNED, RESTARTING :
 * CANCELED -> RESTARTING :
 * COMPLETED -> RESTARTING :
 * FAILED -> RESTARTING :
 * RESTARTING -> NOT_PLANNED :
 */

//! \brief A state machine that manages the state of a UMAA TaskPlanExecution, MissionPlanExecution, or
//    ObjectiveExecution session that has already been issued.
class TaskStateMachine {
 public:
  TaskStateMachine() = default;
  virtual ~TaskStateMachine() = default;

  //! \brief Advance the state machine to the next state if it exists
  //! \return whether the state could be advanced
  bool advanceState();

  //! \brief Change the current state to NOT_PLANNED if the transition is valid
  //! \return whether the state could be updated to NOT_PLANNED
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

  //! \brief Change the current state to FAILED if the transition is valid
  //! \return whether the state could be updated to FAILED
  bool fail();

  //! \brief Change the current state to RESTARTING if the transition is valid
  //! \return whether the state could be updated to RESTARTING
  bool restart();

  //! \brief Gets the current state of the state machine
  //! \return the current state
  TaskStateEnumType getState() const { return currentState_; }

  //! \brief Check if the current state of the state machine is final (i.e. can no longer be updated)
  //! \return whether the current state is considered final
  bool isFinal() const { return isStateFinal(currentState_); }

  //! \brief Helper function to check whether a state is considered final
  //! \param state The state to check
  //! \return whether the provided state is considered final
  static bool isStateFinal(TaskStateEnumType state) {
    // This check is needed because paused isn't a final state, but it also can't advance without being told to resume.
    if (state == TaskStateEnumType::PAUSED) {
      return false;
    }
    return SUCCESS_STATE_MAP_.count(state) == 0;
  }

  //! \brief Resets the state machine to its initial state
  void reset();

 private:
  static const std::map<const TaskStateEnumType, const TaskStateEnumType> SUCCESS_STATE_MAP_;
  TaskStateEnumType currentState_ = TaskStateEnumType::NOT_PLANNED;
};

}  // namespace arlcore::umaa

#endif  // INCLUDE_UMAA_TASKSTATEMACHINE_H_
