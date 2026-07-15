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

#include "ActiveConstraintsControlProvider.h"

namespace arlcore::umaa::conditional {

ActiveConstraintsControlProvider::ActiveConstraintsControlProvider(const NumericGuid& source,
    std::shared_ptr<ActiveConstraintsControlProviderIo> io, bool standingSession) :
    ActiveConstraintsControlProviderBase(source, io),
    Subject<std::vector<std::shared_ptr<ConditionalBase>>>(true),
    standingSession_(standingSession) {}

std::optional<std::vector<std::shared_ptr<ConditionalBase>>>
    ActiveConstraintsControlProvider::getConstraintConditionals() {
  return constraintConditionals_;
}

void ActiveConstraintsControlProvider::update(const std::vector<std::shared_ptr<ConditionalBase>>& data) {
  conditionals_ = data;
  conditionalsDirty_ = true;
}

ReadStatus ActiveConstraintsControlProvider::read(ActiveConstraintsCommandType* outCommand) {
  ReadStatus status = ActiveConstraintsControlProviderBase::read(outCommand);
  while (standingSession_ && status == ReadStatus::DISPOSED) {
    UMAA_LOG_INFO(util::SYSTEM_LOGGER, "Ignoring disposed ActiveConstraints command; the standing session"
      " keeps the applied constraint set")
    status = ActiveConstraintsControlProviderBase::read(outCommand);
  }
  return status;
}

bool ActiveConstraintsControlProvider::isCommandValid(const ActiveConstraintsCommandType& cmd) {
  for (auto it = cmd.constraintConditionalIDs().begin(); it != cmd.constraintConditionalIDs().end(); ++it) {
    if (!conditionalExists(NumericGuid(*it))) {
      UMAA_LOG_WARN(util::SYSTEM_LOGGER, "ActiveConstraintsCommandType contains a constraint conditional ID not"
      " found in the conditional report consumer. Validation failed.")
      lastReason_ = CommandStatusReasonEnumType::VALIDATION_FAILED;
      return false;
    }
  }
  lastReason_ = CommandStatusReasonEnumType::SUCCEEDED;
  return true;
}

CommandStateResult ActiveConstraintsControlProvider::onCommanded(const std::weak_ptr<CmdSession> session) {
  std::set<NumericGuid> ids;
  if (std::shared_ptr<CmdSession> cmdSession = session.lock()) {
    ActiveConstraintsCommandType cmd = cmdSession->getCommand();
    std::transform(cmd.constraintConditionalIDs().begin(), cmd.constraintConditionalIDs().end(),
      std::inserter(ids, ids.begin()), [] (NumericGUID id) { return NumericGuid(id); });
  } else {
    UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Unable to acquire lock on command session")
    return CommandStateResult::ERROR;
  }
  constraintConditionalIds_ = ids;
  conditionalsDirty_ = true;
  return CommandStateResult::ADVANCE;
}

bool ActiveConstraintsControlProvider::isCommandCompleted(const std::weak_ptr<CmdSession> session) {
  std::vector<std::shared_ptr<ConditionalBase>> constraints;
  if (!conditionals_.has_value() || !constraintConditionalIds_.has_value()) {
    return false;
  }

  if (standingSession_ && !conditionalsDirty_) {
    return false;
  }

  std::set<NumericGuid> ids = constraintConditionalIds_.value();

  std::for_each(conditionals_->begin(), conditionals_->end(),
    [&](std::shared_ptr<arlcore::umaa::conditional::ConditionalBase> c) {
      auto result = std::find(ids.begin(), ids.end(), c->getConditionalId());
      if (result != ids.end()) {
        constraints.push_back(c);
        ids.erase(result);
      }
  });

  if (!ids.empty()) {
    if (!standingSession_) {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Missing conditionals from set of active constraints")
      lastReason_ = CommandStatusReasonEnumType::SERVICE_FAILED;
      return false;
    }
    // The IDs stay latched: a conditional deleted while active is deactivated, and re-activates
    // if it is re-added under the same ID (e.g. a commander editing an active constraint).
    UMAA_LOG_WARN(util::SYSTEM_LOGGER, ids.size() << " active constraint conditional(s) missing from the"
      " conditional report; deactivated until re-added")
  }

  constraintConditionals_ = constraints;
  conditionalsDirty_ = false;
  this->notify(constraints);

  return !standingSession_;
}

bool ActiveConstraintsControlProvider::conditionalExists(const NumericGuid& conditionalId) {
  if (!conditionals_.has_value()) {
    return false;
  }

  return std::any_of(conditionals_->begin(), conditionals_->end(),
    [&conditionalId](std::shared_ptr<arlcore::umaa::conditional::ConditionalBase> c) {
      return c->getConditionalId() == conditionalId;
  });
}

}  // namespace arlcore::umaa::conditional
