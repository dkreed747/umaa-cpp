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
    std::shared_ptr<ActiveConstraintsControlProviderIo> io) :
    ActiveConstraintsControlProviderBase(source, io),
    Subject<std::vector<std::shared_ptr<ConditionalBase>>>(true) {}

std::optional<std::vector<std::shared_ptr<ConditionalBase>>>
    ActiveConstraintsControlProvider::getConstraintConditionals() {
  return constraintConditionals_;
}

void ActiveConstraintsControlProvider::update(const std::vector<std::shared_ptr<ConditionalBase>>& data) {
  conditionals_ = data;
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
  return CommandStateResult::ADVANCE;
}

bool ActiveConstraintsControlProvider::isCommandCompleted(const std::weak_ptr<CmdSession> session) {
  std::vector<std::shared_ptr<ConditionalBase>> constraints;
  if (!conditionals_.has_value() || !constraintConditionalIds_.has_value()) {
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
    UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Missing conditionals from set of active constraints")
    lastReason_ = CommandStatusReasonEnumType::SERVICE_FAILED;
    return false;
  }

  constraintConditionals_ = constraints;
  this->notify(constraints);

  return true;
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

