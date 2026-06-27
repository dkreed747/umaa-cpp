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

#ifndef INCLUDE_UMAA_OBJECTIVE_EXECUTORS_CORE_OBJECTIVEBASE_H_
#define INCLUDE_UMAA_OBJECTIVE_EXECUTORS_CORE_OBJECTIVEBASE_H_

#include <memory>
#include <string>
#include <vector>

#include "UMAA/MM/BaseType/ObjectiveDetailedStatusType.hpp"
#include "UMAA/MM/BaseType/ObjectiveType.hpp"
#include "UMAA/MM/ObjectiveExecutorControl/ObjectiveExecutorExecutionStatusReportType.hpp"
#include "UMAA/SA/GlobalPoseStatus/GlobalPoseReportType.hpp"
#include "UMAA/SA/SpeedStatus/SpeedReportType.hpp"
#include "UMAA/SA/VelocityStatus/VelocityReportType.hpp"

#include "Logger.h"
#include "ObjectiveStateMachine.h"
#include "SenderBase.h"
#include "Observer.h"
#include "ConditionalBase.h"

namespace arlcore::umaa {

using UMAA::MM::BaseType::ObjectiveDetailedStatusType;
using UMAA::MM::BaseType::ObjectiveType;
using UMAA::MM::ObjectiveExecutorControl::ObjectiveExecutorExecutionStatusReportType;
using UMAA::Common::Measurement::DateTime;
using UMAA::Common::IdentifierType;
using UMAA::Common::MaritimeEnumeration::ObjectiveExecutorControlEnumModule::ObjectiveExecutorControlEnumType;
using UMAA::MM::BaseType::StateTriggerType;

using UMAA::SA::GlobalPoseStatus::GlobalPoseReportType;
using UMAA::SA::SpeedStatus::SpeedReportType;
using UMAA::SA::VelocityStatus::VelocityReportType;

//! \brief Base class containing all the information shared across all objective types
//! Includes virtual functions that must be overridden for derived objectives to control what should occur on each state
class ObjectiveBase :
  public Observer<std::vector<std::shared_ptr<conditional::ConditionalBase>>>,
  public Observer<GlobalPoseReportType>,
  public Observer<SpeedReportType>,
  public Observer<VelocityReportType> {
 public:
  //! \brief Explicit constructor that takes a umaa base objective type as an input
  //! \param objective UMAA objective base type that carries all common information for an objective
  //! \param exeStatSender IO sender object to send objective execution status report types
  explicit ObjectiveBase(
    const ObjectiveType& objective,
    std::shared_ptr<arlcore::io::SenderBase<ObjectiveExecutorExecutionStatusReportType>> exeStatSender) :
    obj_(objective),
    exeStatSender_(exeStatSender) {}

  //! \brief Virtual deconstructor with default implementation, nothing to clean up in the base class
  virtual ~ObjectiveBase() {}

  //! \brief Implementer defined onCycle should be overridden with any code that needs to be executed regardless of the
  //! objectives current state. onCycle is the first function called at the start of every cycle of the objective
  //! executor.
  //! \return boolean true for success and false for failure
  virtual bool onCycle() = 0;

  //! \brief Implementer defined onQueued should be overridden with anything an objective should do in its QUEUED
  //! state
  //! \return boolean true for success and false for failure
  virtual bool onQueued() = 0;

  //! \brief Implementer defined onCanceled should be overridden with anything an objective should do in its CANCELED
  //! state
  //! \return boolean true for success and false for failure
  virtual bool onCanceled() = 0;

  //! \brief Implementer defined onCanceling should be overridden with anything an objective should do in its CANCELING
  //! state
  //! \return boolean true for success and false for failure
  virtual bool onCanceling() = 0;

  //! \brief Implementer defined onCompleted should be overridden with anything an objective should do in its COMPLETED
  //! state
  //! \return boolean true for success and false for failure
  virtual bool onCompleted() = 0;

  //! \brief Implementer defined onExecuting should be overridden with anything an objective should do in its EXECUTING
  //! state
  //! \return boolean true for success and false for failure
  virtual bool onExecuting() = 0;

  //! \brief Implementer defined onFailed should be overridden with anything an objective should do in its FAILED
  //! state
  //! \return boolean true for success and false for failure
  virtual bool onFailed() = 0;

  //! \brief Implementer defined onModifying should be overridden with anything an objective should do in its MODIFYING
  //! state
  //! \return boolean true for success and false for failure
  virtual bool onModifying() = 0;

  //! \brief Implementer defined onPaused should be overridden with anything an objective should do in its PAUSED
  //! state
  //! \return boolean true for success and false for failure
  virtual bool onPaused() = 0;

  //! \brief Implementer defined onPausing should be overridden with anything an objective should do in its PAUSING
  //! state
  //! \return boolean true for success and false for failure
  virtual bool onPausing() = 0;

  //! \brief Implementer defined onResuming should be overridden with anything an objective should do in its RESUMING
  //! state
  //! \return boolean true for success and false for failure
  virtual bool onResuming() = 0;

  //! \brief Implementer defined isObjectiveValid should be overridden with logic to determine if the objective is valid
  //! or not
  //! \return boolean true for valid and false for invalid
  virtual bool isObjectiveValid() = 0;

  //! \brief Implementer defined isObjectiveComplete should be overridden with logic to determine if the objective has
  //! been completed or not
  //! \return boolean true for complete and false for incomplete
  virtual bool isObjectiveComplete() = 0;

  //! \brief Implementer defined isObjectiveFailed should be overridden with logic to determine if the objective has
  //! failed or not
  //! \return ObjectiveExecutorStateReasonEnumType: Any failure enum to indicate failure reason else SUCCEEDED for
  //! objective still nominal
  virtual ObjectiveExecutorStateReasonEnumType isObjectiveFailed() = 0;

  //! \brief Implementer defined function to send an ObjectiveExecutorExecutionStatusReportType
  //! \return
  virtual arlcore::io::SendStatus sendExecutionStatus() = 0;

  //! \brief Get the boolean for whether approvalRequired is required for the objective
  //! \return boolean isApprovalRequired
  bool isApprovalRequired() const {
    return obj_.approvalRequired();
  }

  //! \brief Get NumericGUID of the optional value duringCondition
  //! \return std::optional<arlcore::NumericGuid>
  std::optional<arlcore::NumericGuid> getDuringConditionID() const {
    return obj_.duringConditionID().has_value() ?
      std::optional<arlcore::NumericGuid>(obj_.duringConditionID().value()) :
      std::nullopt;
  }

  //! \brief Get the name of the objective
  //! \return string name
  std::string getName() const {
    return obj_.name();
  }

  //! \brief Get the description of the objective
  //! \return string description
  std::string getDescription() const {
    return obj_.objectiveDescription();
  }

  //! \brief Get the GUID of the objective
  //! \return NumericGuid Objective ID
  arlcore::NumericGuid getId() const {
    return arlcore::NumericGuid(obj_.objectiveID());
  }

  //! \brief Get the priority of the objective
  //! 0 - Lowest priority
  //! 255 - Highest priority
  //! \return integer priority
  int32_t getPriority() const {
    return obj_.objectivePriority();
  }

  //! \brief Get the optional NumericGUID of the preconditional ID
  //! \return std::optional<arlcore::NumericGuid>
  std::optional<arlcore::NumericGuid> getPreconditionID() const {
    return obj_.preconditionID().has_value() ?
      std::optional<arlcore::NumericGuid>(obj_.preconditionID().value()) :
      std::nullopt;
  }

  //! \brief Get the IDs for the preferred resources to be used by the objective
  //! \return std::vector of identifier types
  std::vector<IdentifierType> getPreferredResources() const {
    std::vector<IdentifierType> internalType;
    for (auto id = obj_.preferredResourceID().begin(); id != obj_.preferredResourceID().end(); id++) {
      internalType.push_back(*id);
    }
    return internalType;
  }

  //! \brief Get the NumericGuid of the specializationID type this objective is linked to
  //! \return arlcore::NumericGuid
  arlcore::NumericGuid getSpecializationID() const {
    return arlcore::NumericGuid(obj_.specializationID());
  }

  //! \brief Get the timestamp associated with the atomic updates for the specialization type
  //! \return UMAA DateTime type
  DateTime getSpecializationTimestamp() const {
    return obj_.specializationTimestamp();
  }

  //! \brief Get the topic string that the related specialization type is published on
  //! \return std::string topic
  std::string getSpecializationTopic() const {
    return obj_.specializationTopic();
  }

  //! \brief Get the state triggers associated with the objective
  //! \return std::vector of UMAA State Trigger types
  std::vector<StateTriggerType> getStateTriggers() const {
    std::vector<StateTriggerType> internalType;
    for (auto trigger = obj_.stateTrigger().begin(); trigger != obj_.stateTrigger().end(); trigger++) {
      internalType.push_back(*trigger);
    }
    return internalType;
  }

  //! \brief Advance the state along the normal flow of an objective QUEUED --> EXECUTING --> COMPLETED
  //! \return boolean true if state advancement was successful else false
  bool advanceObjectiveState() { return stateMachine_.advanceState(); }

  //! \brief Transition the objective state to the FAILED state
  //! \return boolean true if state advancement was successful else false
  bool transitionObjectiveStateToFailed(const ObjectiveExecutorStateReasonEnumType& reason) {
    return stateMachine_.fail(reason);
  }

  //! \brief Transition the objective state to the CANCELING state
  //! \return boolean true if state advancement was successful else false
  bool transitionObjectiveStateToCanceled() { return stateMachine_.cancel(); }

  //! \brief Transition the objective state to the Modifying state
  //! \return boolean true if state advancement was successful else false
  bool transitionObjectiveStateToModifying() { return stateMachine_.update(); }

  //! \brief Get the current state of the stored objective
  //! \return ObjectiveExecutorStateEnumType internal objective status enum type
  ObjectiveExecutorStateEnumType getObjectiveState() const { return stateMachine_.getState(); }

  //! \brief Get the current state reason of the stored objective
  //! \return ObjectiveExecutorStateReasonEnumType internal objective status reason enum type
  ObjectiveExecutorStateReasonEnumType getObjectiveStateReason() const { return stateMachine_.getReason(); }

  //! \brief Check if the stored objective is in a terminal state
  //! \return boolean true if the state is final else false
  bool isObjectiveInTerminalState() const { return stateMachine_.isFinal(); }

  //! \brief Transition the objective state according to a state change command (EXECUTE, PAUSE, or RESUME)
  //! \param desiredState UMAA ObjectiveExecutorControlEnumType holding the state that should be transitioned to
  //! \return boolean true if transition was successful else false
  bool commandObjectiveState(const ObjectiveExecutorControlEnumType& desiredState) {
    bool retSuccess = true;
    switch (desiredState) {
      case ObjectiveExecutorControlEnumType::EXECUTE:
        retSuccess = getObjectiveState() == ObjectiveExecutorStateEnumType::QUEUED ?
          advanceObjectiveState() : false;
        break;
      case ObjectiveExecutorControlEnumType::PAUSE:
        retSuccess = stateMachine_.pause();
        break;
      case ObjectiveExecutorControlEnumType::RESUME:
        retSuccess = stateMachine_.resume();
        break;
      default:
        UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Tried to handle unsupported ObjectiveExecutorControlEnumType: " <<
          desiredState);
        retSuccess = false;
    }
    return retSuccess;
  }

  //! \brief Overridden function to update the active constraints via Observer
  //! \param data The updated active constraint data
  void update(const std::vector<std::shared_ptr<conditional::ConditionalBase>>& data) override {
    activeConstraints_ = data;
  }

  void update(const GlobalPoseReportType& data) override {
    gpReport_ = data;
  }

  void update(const SpeedReportType& data) override {
    sReport_ = data;
  }

  void update(const VelocityReportType& data) override {
    vReport_ = data;
  }

  std::optional<GlobalPoseReportType> getGlobalPoseReport() const {
    return gpReport_;
  }

  std::optional<SpeedReportType> getSpeedReport() const {
    return sReport_;
  }

  std::optional<VelocityReportType> getVelocityReport() const {
    return vReport_;
  }

 protected:
  ObjectiveType obj_;

  std::shared_ptr<arlcore::io::SenderBase<ObjectiveExecutorExecutionStatusReportType>> exeStatSender_;

  ObjectiveDetailedStatusType objDetailedStatus_;
  arlcore::umaa::ObjectiveStateMachine stateMachine_;
  std::optional<std::vector<std::shared_ptr<conditional::ConditionalBase>>> activeConstraints_;

 private:
  std::optional<GlobalPoseReportType> gpReport_;
  std::optional<SpeedReportType> sReport_;
  std::optional<VelocityReportType> vReport_;
};

}  // namespace arlcore::umaa
#endif  // INCLUDE_UMAA_OBJECTIVE_EXECUTORS_CORE_OBJECTIVEBASE_H_
