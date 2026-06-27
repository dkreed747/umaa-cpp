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

#ifndef INCLUDE_UMAA_OBJECTIVE_EXECUTORS_SERVICES_OBJECTIVEEXECUTORCONTROLSERVICEPROVIDER_H_
#define INCLUDE_UMAA_OBJECTIVE_EXECUTORS_SERVICES_OBJECTIVEEXECUTORCONTROLSERVICEPROVIDER_H_

// System Headers
#include <list>
#include <memory>

// ARL Headers
#include "CommandProviderBase.h"
#include "ObjectiveExecutorControlServiceProviderIo.h"
#include "ObjectiveFactory.h"
#include "UmaaUtils.h"
#include "ActiveConstraintsControlProvider.h"

namespace arlcore::umaa {

class ObjectiveExecutorControlServiceProvider : public arlcore::umaa::services::CommandProviderBase<
  ObjectiveExecutorCommandType,
  ObjectiveExecutorCommandAckReportType,
  ObjectiveExecutorCommandStatusType,
  ObjectiveExecutorExecutionStatusReportType> {
 public:
  //! \brief Delete default constructor
  ObjectiveExecutorControlServiceProvider() = delete;

  ObjectiveExecutorControlServiceProvider (const ObjectiveExecutorControlServiceProvider&) = delete;
  ObjectiveExecutorControlServiceProvider& operator=(const ObjectiveExecutorControlServiceProvider&) = delete;

  //! \brief Main constructor for the Objective Executor Provider using the vector control service
  //! \param source Source ID of the provider
  //! \param io io object containing pointers to concrete readers and writers
  //! \param objFactory pointer to a concrete objective factory to use when building new objectives
  ObjectiveExecutorControlServiceProvider(
    const NumericGuid& source,
    std::shared_ptr<ObjectiveExecutorControlServiceProviderIo> io,
    std::shared_ptr<ObjectiveFactory> objFactory);

  //! \brief Destructor to reset static values
  ~ObjectiveExecutorControlServiceProvider();

  //! \brief Overridden onCycle from base provider with objective specific logic we want to run every provider cycle
  //! Functions:
  //!   1. Reset active objective if command goes terminal
  //! \return bool success/failure
  bool onCycle() override;

  //! \brief Overridden onCanceled from base provider with objective specific logic we want to run when the consumer
  //! cancels the command
  //! Functions:
  //!   1. Change the active objective state to CANCELING
  //! \return bool success/failure
  bool onCanceled(const std::weak_ptr<CmdSession> session) override;

  //! \brief Overridden onUpdated from base provider with objective specific logic we want to run when the consumer
  //! updates a command
  //!   1. Transition objective state to MODIFYING
  //! \return
  bool onUpdated(const std::weak_ptr<CmdSession> session, const ObjectiveExecutorCommandType& previousCmd,
    const ObjectiveExecutorCommandType& updatedCmd) override;

  //! \brief Overridden isCommandValid from base provider to add objective specific validation logic on incoming
  //! commands to determine if it should be allowed to transition to the COMMANDED state.
  //! Functions:
  //!   1. Build drift objective type
  //! \param cmd constant reference to the command to be validated
  //! \return bool success/failure
  bool isCommandValid(const ObjectiveExecutorCommandType& cmd) override;

  //! \brief Overridden isCommandCompleted from base provider to add Objective specific completion logic on executing
  //! commands to determine when a command should transition from the EXECUTING to COMPLETED state.
  //! Conditions: If objective is complete
  //! \return bool success/failure
  bool isCommandCompleted(const std::weak_ptr<CmdSession> session) override;

  //! \brief Overridden isCommandFailed from base provider to add Objective specific failed logic on executing
  //! commands to determine when a command should transition from the EXECUTING to FAILED state.
  //! Conditions: If objective is failed
  //! \return CommandStatusReasonEnumType reason for failure
  CommandStatusReasonEnumType isCommandFailed(const std::weak_ptr<CmdSession> session) override;

  //! \brief Overridden sendExecutionStatus to send objective specific status messages during the execution phase
  //! \return SendStatus enum type
  SendStatus sendExecutionStatus(const ObjectiveExecutorCommandType& cmd) override;

  //! \brief Overridden disposeExecutionStatus handles cleanup of execution status related messages
  //! \return SendStatus enum type
  SendStatus disposeExecutionStatus(const ObjectiveExecutorCommandType& cmd) override;

  //! \brief Sets the ActiveConstraintsControlProvider to register observers to when objectives are created
  //! \param constraintProvider The ActiveConstraintsControlProvider to register objectives to as observers
  void setActiveConstraintsProvider(
    std::optional<std::shared_ptr<conditional::ActiveConstraintsControlProvider>> constraintProvider);

  //! \brief Get the active route objective
  //! \return potentially empty optional if provider doesn't have active objective
  static std::optional<std::shared_ptr<ObjectiveBase>> getActiveObjective();

 private:
  NumericGuid sourceId_;
  std::shared_ptr<ObjectiveFactory> objFactory_;
  std::optional<std::shared_ptr<conditional::ActiveConstraintsControlProvider>> constraintProvider_;

  static std::optional<std::shared_ptr<ObjectiveBase>> activeObjective_;

  //! \brief Unregister the active objective from the ActiveConstraintsControlProvider if both exist
  void unregisterActiveObjective() {
    if (activeObjective_.has_value() && constraintProvider_.has_value()) {
      constraintProvider_.value()->unregisterObserver(activeObjective_.value());
    }
  }
};

}  // namespace arlcore::umaa
#endif  // INCLUDE_UMAA_OBJECTIVE_EXECUTORS_SERVICES_OBJECTIVEEXECUTORCONTROLSERVICEPROVIDER_H_
