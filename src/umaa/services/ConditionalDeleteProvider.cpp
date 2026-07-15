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

#include "ConditionalDeleteProvider.h"

#include <vector>

namespace arlcore::umaa::conditional {

ConditionalDeleteProvider::ConditionalDeleteProvider(
    const NumericGuid& source,
    std::shared_ptr<ConditionalDeleteProviderIo> io,
    std::shared_ptr<ConditionalReportProvider> reportProvider) :
    ConditionalDeleteProviderBase(source, io, services::IncomingCommandBehavior::QUEUE_INCOMING),
    reportProvider_(reportProvider) {}

bool ConditionalDeleteProvider::isCommandValid(const ConditionalDeleteCommandType& cmd) {
  if (!cmd.conditionalID().has_value()) {
    return true;
  }

  if (!reportProvider_->getConditionalById(NumericGuid(cmd.conditionalID().value())).has_value()) {
    UMAA_LOG_WARN(util::SYSTEM_LOGGER, "ConditionalDeleteCommand references unknown conditionalID: " <<
      NumericGuid(cmd.conditionalID().value()) << ". Validation failed.")
    return false;
  }

  return true;
}

CommandStateResult ConditionalDeleteProvider::onCommanded(const std::weak_ptr<CmdSession> session) {
  std::shared_ptr<CmdSession> cmdSession = session.lock();
  if (!cmdSession) {
    UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Unable to acquire lock on command session")
    return CommandStateResult::ERROR;
  }

  ConditionalDeleteCommandType cmd = cmdSession->getCommand();

  if (cmd.conditionalID().has_value()) {
    if (reportProvider_->removeSpecialization(NumericGuid(cmd.conditionalID().value())) != SendStatus::SUCCESS) {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Failed to remove conditional " <<
        NumericGuid(cmd.conditionalID().value()) << " from the working report")
      return CommandStateResult::ERROR;
    }
  } else {
    std::vector<ConditionalType> conditionals = reportProvider_->getConditionals();
    for (const ConditionalType& conditional : conditionals) {
      if (reportProvider_->removeSpecialization(NumericGuid(conditional.conditionalID())) != SendStatus::SUCCESS) {
        UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Failed to remove conditional " <<
          NumericGuid(conditional.conditionalID()) << " while deleting all conditionals")
        return CommandStateResult::ERROR;
      }
    }
  }

  if (reportProvider_->sendReport() != SendStatus::SUCCESS) {
    UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Failed to send conditional report after delete")
    return CommandStateResult::ERROR;
  }

  return CommandStateResult::ADVANCE;
}

bool ConditionalDeleteProvider::isCommandCompleted(const std::weak_ptr<CmdSession> session) {
  return true;
}

}  // namespace arlcore::umaa::conditional
