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

#ifndef INCLUDE_UMAA_OBJECTIVE_EXECUTORS_SERVICES_OBJECTIVEEXECUTORSTATECONTROLSERVICEPROVIDER_H_
#define INCLUDE_UMAA_OBJECTIVE_EXECUTORS_SERVICES_OBJECTIVEEXECUTORSTATECONTROLSERVICEPROVIDER_H_

// System Headers
#include <functional>
#include <memory>
#include <optional>

// ARL Headers
#include "CommandProviderBase.h"
#include "ObjectiveBase.h"
#include "ObjectiveExecutorStateControlServiceProviderIo.h"

using arlcore::umaa::services::CommandStateResult;

namespace arlcore::umaa {

class ObjectiveExecutorStateControlServiceProvider : public arlcore::umaa::services::CommandProviderBase<
  ObjectiveExecutorStateCommandType,
  ObjectiveExecutorStateCommandAckReportType,
  ObjectiveExecutorStateCommandStatusType,
  ObjectiveExecutorExecutionStatusReportType> {
 public:
  ObjectiveExecutorStateControlServiceProvider(
    const NumericGuid& source,
    std::shared_ptr<ObjectiveExecutorStateControlServiceProviderIo> io,
    const std::function<std::optional<std::shared_ptr<ObjectiveBase>>()>& getActiveObjectiveCallback);

  //! \brief Overridden isCommandValid to check if the objective referenced by the command exists
  //! \return bool doesCommandReferenceValidObjective
  bool isCommandValid(const ObjectiveExecutorStateCommandType& cmd) override;

  //! \brief Overridden onExecuting to begin transitioning objective state to desired state
  //! \return bool success/failure
  CommandStateResult onExecuting(const std::weak_ptr<CmdSession> session) override;

  //! \brief Overridden isCommandCompleted to check if the commanded objective has reached the desired state
  //! \return bool isObjectiveInDesiredState
  bool isCommandCompleted(const std::weak_ptr<CmdSession> session) override;

  //! \brief Overridden isCommandFailed to check if the state command has failed
  //! \return CommandStatusReasonEnumType
  CommandStatusReasonEnumType isCommandFailed(const std::weak_ptr<CmdSession> session) override;

 private:
  std::function<std::optional<std::shared_ptr<ObjectiveBase>>()> getActiveObjectiveCallback_;
};

}  // namespace arlcore::umaa
#endif  // INCLUDE_UMAA_OBJECTIVE_EXECUTORS_SERVICES_OBJECTIVEEXECUTORSTATECONTROLSERVICEPROVIDER_H_
