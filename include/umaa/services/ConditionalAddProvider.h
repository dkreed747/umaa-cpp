//---------------------------------------------------------------------------
// Copyright 2026 Pennsylvania State University
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

#ifndef INCLUDE_UMAA_SERVICES_CONDITIONALADDPROVIDER_H_
#define INCLUDE_UMAA_SERVICES_CONDITIONALADDPROVIDER_H_

#include <map>
#include <memory>
#include <set>
#include <string>

#include <UMAA/MM/ConditionalControl/ConditionalAddCommandType.hpp>
#include <UMAA/MM/ConditionalControl/ConditionalAddCommandStatusType.hpp>
#include <UMAA/MM/ConditionalControl/ConditionalAddCommandAckReportType.hpp>

#include "CommandProviderBase.h"
#include "ConditionalFactoryIo.h"
#include "ConditionalReportProvider.h"

using UMAA::MM::ConditionalControl::ConditionalAddCommandType;
using UMAA::MM::ConditionalControl::ConditionalAddCommandStatusType;
using UMAA::MM::ConditionalControl::ConditionalAddCommandAckReportType;

using ConditionalAddProviderBase =
  arlcore::umaa::services::CommandProviderBase<ConditionalAddCommandType, ConditionalAddCommandAckReportType,
  ConditionalAddCommandStatusType>;

using ConditionalAddProviderIo = UmaaCommandProviderIo<ConditionalAddCommandType,
    ConditionalAddCommandAckReportType, ConditionalAddCommandStatusType>;

using arlcore::umaa::services::CommandStateResult;

namespace arlcore::umaa::conditional {

//! \brief A provider for the UMAA MM ConditionalControl Add command.
//!
//! An Add command carries a generic ConditionalType whose specializationTopic/specializationID reference a
//! specialization payload the commander publishes on the matching specialization topic under its own writer. This
//! provider resolves that payload through the shared ConditionalFactoryIo specialization caches (waiting up to
//! maxSpecializationWaitCycles cycles for it to arrive), then re-publishes it under the ConditionalReportProvider's
//! writer with a fresh specializationReferenceID so the payload instance survives the commander's writer. The
//! commander-minted conditionalID is preserved: an Add whose conditionalID already exists in the report is an
//! upsert (the superseded payload instance is disposed).
class ConditionalAddProvider : public ConditionalAddProviderBase {
 public:
  //! \brief Constructor
  //! \param source The source ID to receive commands for and sign session messages with
  //! \param io Command provider io containing the command reader and ack/status writers
  //! \param reportProvider The conditional report provider that owns the working conditional set
  //! \param factoryIo The factory io whose specialization caches resolve commanded payloads
  //! \param supportedTopics Specialization topics accepted by this provider; empty accepts every known topic
  //! \param maxSpecializationWaitCycles Cycles to wait for a commanded payload before failing with TIMEOUT
  ConditionalAddProvider(
    const NumericGuid& source,
    std::shared_ptr<ConditionalAddProviderIo> io,
    std::shared_ptr<ConditionalReportProvider> reportProvider,
    std::shared_ptr<ConditionalFactoryIo> factoryIo,
    std::set<std::string> supportedTopics = {},
    uint32_t maxSpecializationWaitCycles = 100);

 private:
  //! \brief Per-session resolution progress for the commanded specialization payload
  struct PendingAdd {
    uint32_t waitCycles = 0;
    bool published = false;
    CommandStatusReasonEnumType failReason = CommandStatusReasonEnumType::SUCCEEDED;
  };

  std::shared_ptr<ConditionalReportProvider> reportProvider_;
  std::shared_ptr<ConditionalFactoryIo> factoryIo_;
  std::set<std::string> supportedTopics_;
  uint32_t maxWaitCycles_;
  std::map<NumericGuid, PendingAdd> pending_;

  //! @brief Overridden function to determine whether a given command is valid
  //! @param cmd The command to validate
  //! @return Whether the provided command is valid
  bool isCommandValid(const ConditionalAddCommandType& cmd) override;

  //! @brief Overridden function that resolves the commanded payload and publishes it into the conditional report
  //! @return OK while waiting for the payload, ADVANCE once it is published, ERROR on publish failure
  CommandStateResult onExecuting(const std::weak_ptr<CmdSession> session) override;

  //! @brief Overridden function to determine whether the active command has been completed
  //! @return Whether the commanded conditional has been published into the report
  bool isCommandCompleted(const std::weak_ptr<CmdSession> session) override;

  //! @brief Overridden method used to determine if the active command is in a failed state
  //! @return The failure reason for the session; SUCCEEDED while the command is healthy
  CommandStatusReasonEnumType isCommandFailed(const std::weak_ptr<CmdSession> session) override;

  //! @brief Overridden function to clean up per-session state on completion
  bool onCompleted(const std::weak_ptr<CmdSession> session) override;

  //! @brief Overridden function to clean up per-session state on cancellation
  bool onCanceled(const std::weak_ptr<CmdSession> session) override;

  //! @brief Overridden function to clean up per-session state on failure
  bool onFailed(const std::weak_ptr<CmdSession> session) override;

  //! \brief Drop the per-session resolution state for a session
  void erasePending(const std::weak_ptr<CmdSession> session);

  //! \brief Route the requested conditional to the specialization cache matching its specializationTopic
  //! \param requested The generic conditional from the Add command
  //! \return OK if the payload has not arrived yet, ADVANCE once published, ERROR on failure
  CommandStateResult dispatchResolve(const ConditionalType& requested);

  //! \brief Resolve the commanded payload from a cache and re-publish it under the report provider's writer
  //! with a fresh specializationReferenceID, preserving the commander-minted conditionalID (upsert)
  //! \tparam Specialized The UMAA specialized conditional type
  //! \param cache The specialization cache to resolve the payload from
  //! \param requested The generic conditional from the Add command
  //! \return OK if the payload has not arrived yet, ADVANCE once published, ERROR on failure
  template <class Specialized>
  CommandStateResult resolveAndPublish(SpecializationCache<Specialized>* cache, const ConditionalType& requested);
};

}  // namespace arlcore::umaa::conditional
#endif  // INCLUDE_UMAA_SERVICES_CONDITIONALADDPROVIDER_H_
