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

#ifndef INCLUDE_UMAA_SERVICES_ACTIVECONSTRAINTSCONTROLPROVIDER_H_
#define INCLUDE_UMAA_SERVICES_ACTIVECONSTRAINTSCONTROLPROVIDER_H_

#include <memory>
#include <set>
#include <vector>

#include <UMAA/MM/ActiveConstraintsControl/ActiveConstraintsCommandType.hpp>
#include <UMAA/MM/ActiveConstraintsControl/ActiveConstraintsCommandStatusType.hpp>
#include <UMAA/MM/ActiveConstraintsControl/ActiveConstraintsCommandAckReportType.hpp>

#include "CommandProviderBase.h"
#include "ConditionalReportConsumer.h"
#include "Observer.h"
#include "Subject.h"

using UMAA::MM::ActiveConstraintsControl::ActiveConstraintsCommandType;
using UMAA::MM::ActiveConstraintsControl::ActiveConstraintsCommandStatusType;
using UMAA::MM::ActiveConstraintsControl::ActiveConstraintsCommandAckReportType;

using ActiveConstraintsControlProviderBase =
  arlcore::umaa::services::CommandProviderBase<ActiveConstraintsCommandType, ActiveConstraintsCommandAckReportType,
  ActiveConstraintsCommandStatusType>;

using ActiveConstraintsControlProviderIo = UmaaCommandProviderIo<ActiveConstraintsCommandType,
    ActiveConstraintsCommandAckReportType, ActiveConstraintsCommandStatusType>;

using arlcore::umaa::services::CommandStateResult;

namespace arlcore::umaa::conditional {

class ActiveConstraintsControlProvider :
    public ActiveConstraintsControlProviderBase,
    public Subject<std::vector<std::shared_ptr<ConditionalBase>>>,
    public Observer<std::vector<std::shared_ptr<ConditionalBase>>> {
 public:
  //! \brief Constructor
  //! \param source The source ID to receive commands for and sign session messages with
  //! \param io Command provider io containing the command reader and ack/status writers
  //! \param standingSession When true the accepted command session stays EXECUTING indefinitely: the applied
  //! constraint set outlives the commander (a disposed command is ignored rather than treated as a cancel, so a
  //! commander restart's writer autodispose cannot silently clear the set), a new command supersedes the standing
  //! one (exactly one live ack mirrors the applied set), and active conditionals are re-resolved by ID on every
  //! conditional set change (a deleted conditional is deactivated with a warning instead of failing the session,
  //! and re-activates if re-added under the same ID)
  ActiveConstraintsControlProvider(const NumericGuid& source, std::shared_ptr<ActiveConstraintsControlProviderIo> io,
    bool standingSession = false);

  //! \brief Get a list of the currently active constraint conditionals if they exist
  //! \return An optional vector of shared pointers to conditional objects
  std::optional<std::vector<std::shared_ptr<ConditionalBase>>> getConstraintConditionals();

  //! \brief Overridden function used to update the observer with conditional objects from the conditional report
  //! \param data The conditional objects being updated
  void update(const std::vector<std::shared_ptr<ConditionalBase>>& data) override;

  // //! \brief Send the current list of constraints to all observers again.
  //             Allows new observers to get the latest data.
  // void resendConstraints();

 private:
  std::optional<std::vector<std::shared_ptr<ConditionalBase>>> conditionals_;
  std::optional<std::vector<std::shared_ptr<ConditionalBase>>> constraintConditionals_;
  std::optional<std::set<NumericGuid>> constraintConditionalIds_;
  CommandStatusReasonEnumType lastReason_ = CommandStatusReasonEnumType::SUCCEEDED;
  bool standingSession_ = false;
  bool conditionalsDirty_ = false;

  //! @brief Overridden function to read incoming commands. In standing-session mode DISPOSED command samples are
  //! swallowed so the standing session (and its applied constraint set) survives the commander's writer.
  //! @param outCommand pointer to the command data to overwrite
  //! @return ReadStatus enum
  ReadStatus read(ActiveConstraintsCommandType* outCommand) override;

  //! @brief Overridden function for logic that runs when the active command reaches the `COMMANDED` state
  //! @return Whether the custom logic completed successfully
  CommandStateResult onCommanded(const std::weak_ptr<CmdSession> session) override;

  //! @brief Overridden function to determine whether a given command is valid
  //! @param cmd The command to validate
  //! @return Whether the provided command is valid
  bool isCommandValid(const ActiveConstraintsCommandType& cmd) override;

  //! @brief Overridden function to determine whether the active command has been completed
  //! @return Whether the active command has been completed
  bool isCommandCompleted(const std::weak_ptr<CmdSession> session) override;

  bool conditionalExists(const NumericGuid &conditionalId);

  //! @brief Overridden method used to determine if the active command is in a failed state by its latest status reason
  //! @return The status reason of the active command. Values other than `SUCCEEDED` are considered failed.
  CommandStatusReasonEnumType isCommandFailed(const std::weak_ptr<CmdSession> session) override {
    return lastReason_;
  }
};

}  // namespace arlcore::umaa::conditional
#endif  // INCLUDE_UMAA_SERVICES_ACTIVECONSTRAINTSCONTROLPROVIDER_H_
