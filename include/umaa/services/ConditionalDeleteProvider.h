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

#ifndef INCLUDE_UMAA_SERVICES_CONDITIONALDELETEPROVIDER_H_
#define INCLUDE_UMAA_SERVICES_CONDITIONALDELETEPROVIDER_H_

#include <memory>

#include <UMAA/MM/ConditionalControl/ConditionalDeleteCommandType.hpp>
#include <UMAA/MM/ConditionalControl/ConditionalDeleteCommandStatusType.hpp>
#include <UMAA/MM/ConditionalControl/ConditionalDeleteCommandAckReportType.hpp>

#include "CommandProviderBase.h"
#include "ConditionalReportProvider.h"

using UMAA::MM::ConditionalControl::ConditionalDeleteCommandType;
using UMAA::MM::ConditionalControl::ConditionalDeleteCommandStatusType;
using UMAA::MM::ConditionalControl::ConditionalDeleteCommandAckReportType;

using ConditionalDeleteProviderBase =
  arlcore::umaa::services::CommandProviderBase<ConditionalDeleteCommandType, ConditionalDeleteCommandAckReportType,
  ConditionalDeleteCommandStatusType>;

using ConditionalDeleteProviderIo = UmaaCommandProviderIo<ConditionalDeleteCommandType,
    ConditionalDeleteCommandAckReportType, ConditionalDeleteCommandStatusType>;

using arlcore::umaa::services::CommandStateResult;

namespace arlcore::umaa::conditional {

//! \brief A provider for the UMAA MM ConditionalControl Delete command.
//!
//! Deletes the conditional matching the commanded conditionalID from the working conditional report, disposing its
//! specialization payload instance. A command with no conditionalID deletes every conditional in the report. A
//! command referencing an unknown conditionalID fails validation.
class ConditionalDeleteProvider : public ConditionalDeleteProviderBase {
 public:
  //! \brief Constructor
  //! \param source The source ID to receive commands for and sign session messages with
  //! \param io Command provider io containing the command reader and ack/status writers
  //! \param reportProvider The conditional report provider that owns the working conditional set
  ConditionalDeleteProvider(
    const NumericGuid& source,
    std::shared_ptr<ConditionalDeleteProviderIo> io,
    std::shared_ptr<ConditionalReportProvider> reportProvider);

 private:
  std::shared_ptr<ConditionalReportProvider> reportProvider_;

  //! @brief Overridden function to determine whether a given command is valid
  //! @param cmd The command to validate
  //! @return Whether the commanded conditionalID (if any) exists in the working report
  bool isCommandValid(const ConditionalDeleteCommandType& cmd) override;

  //! @brief Overridden function that removes the commanded conditional(s) from the working report
  //! @return ADVANCE on success, ERROR on removal failure
  CommandStateResult onCommanded(const std::weak_ptr<CmdSession> session) override;

  //! @brief Overridden function to determine whether the active command has been completed
  //! @return Always true; the delete is applied while the command is COMMANDED
  bool isCommandCompleted(const std::weak_ptr<CmdSession> session) override;
};

}  // namespace arlcore::umaa::conditional
#endif  // INCLUDE_UMAA_SERVICES_CONDITIONALDELETEPROVIDER_H_
