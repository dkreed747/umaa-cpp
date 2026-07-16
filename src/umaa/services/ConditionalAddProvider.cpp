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

#include "ConditionalAddProvider.h"

#include <utility>

namespace arlcore::umaa::conditional {

namespace {

const std::set<std::string> KNOWN_SPECIALIZATION_TOPICS = {
  UMAA::MM::Conditional::ConstraintViolatedConditionalTypeTopic,
  UMAA::MM::Conditional::DepthConditionalTypeTopic,
  UMAA::MM::Conditional::DepthRateConditionalTypeTopic,
  UMAA::MM::Conditional::EmitterPresetConditionalTypeTopic,
  UMAA::MM::Conditional::ExpConditionalTypeTopic,
  UMAA::MM::Conditional::HeadingSectorConditionalTypeTopic,
  UMAA::MM::Conditional::LogicalANDConditionalTypeTopic,
  UMAA::MM::Conditional::LogicalNOTConditionalTypeTopic,
  UMAA::MM::Conditional::LogicalORConditionalTypeTopic,
  UMAA::MM::Conditional::MissionStateConditionalTypeTopic,
  UMAA::MM::Conditional::ObjectiveStateConditionalTypeTopic,
  UMAA::MM::Conditional::PitchRateConditionalTypeTopic,
  UMAA::MM::Conditional::RelativeSpeedConditionalTypeTopic,
  UMAA::MM::Conditional::RollRateConditionalTypeTopic,
  UMAA::MM::Conditional::SpeedConditionalTypeTopic,
  UMAA::MM::Conditional::TaskStateConditionalTypeTopic,
  UMAA::MM::Conditional::TimeConditionalTypeTopic,
  UMAA::MM::Conditional::WaterZoneConditionalTypeTopic,
  UMAA::MM::Conditional::YawRateConditionalTypeTopic,
};

}  // namespace

ConditionalAddProvider::ConditionalAddProvider(
    const NumericGuid& source,
    std::shared_ptr<ConditionalAddProviderIo> io,
    std::shared_ptr<ConditionalReportProvider> reportProvider,
    std::shared_ptr<ConditionalFactoryIo> factoryIo,
    std::set<std::string> supportedTopics,
    uint32_t maxSpecializationWaitCycles) :
    ConditionalAddProviderBase(source, io, services::IncomingCommandBehavior::QUEUE_INCOMING),
    reportProvider_(reportProvider),
    factoryIo_(factoryIo),
    supportedTopics_(std::move(supportedTopics)),
    maxWaitCycles_(maxSpecializationWaitCycles) {}

bool ConditionalAddProvider::isCommandValid(const ConditionalAddCommandType& cmd) {
  const std::string& topic = cmd.conditional().specializationTopic();

  if (NumericGuid(cmd.conditional().conditionalID()) == NIL_GUID) {
    UMAA_LOG_WARN(util::SYSTEM_LOGGER, "ConditionalAddCommand has a nil conditionalID. Validation failed.")
    return false;
  }

  if (KNOWN_SPECIALIZATION_TOPICS.count(topic) == 0) {
    UMAA_LOG_WARN(util::SYSTEM_LOGGER, "ConditionalAddCommand references unknown specialization topic: " << topic)
    return false;
  }

  if (!supportedTopics_.empty() && supportedTopics_.count(topic) == 0) {
    UMAA_LOG_WARN(util::SYSTEM_LOGGER, "ConditionalAddCommand specialization topic not supported by this provider: "
      << topic)
    return false;
  }

  return true;
}

CommandStateResult ConditionalAddProvider::onExecuting(const std::weak_ptr<CmdSession> session) {
  std::shared_ptr<CmdSession> cmdSession = session.lock();
  if (!cmdSession) {
    UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Unable to acquire lock on command session")
    return CommandStateResult::ERROR;
  }

  PendingAdd& state = pending_[cmdSession->getSessionId()];
  if (state.published || state.failReason != CommandStatusReasonEnumType::SUCCEEDED) {
    return CommandStateResult::OK;
  }

  CommandStateResult result = dispatchResolve(cmdSession->getCommand().conditional());
  if (result == CommandStateResult::ADVANCE) {
    state.published = true;
  } else if (result == CommandStateResult::OK && ++state.waitCycles > maxWaitCycles_) {
    UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Timed out waiting for specialization payload of conditional " <<
      NumericGuid(cmdSession->getCommand().conditional().conditionalID()))
    state.failReason = CommandStatusReasonEnumType::TIMEOUT;
    return CommandStateResult::OK;
  }
  return result;
}

bool ConditionalAddProvider::isCommandCompleted(const std::weak_ptr<CmdSession> session) {
  std::shared_ptr<CmdSession> cmdSession = session.lock();
  if (!cmdSession) {
    return false;
  }
  auto it = pending_.find(cmdSession->getSessionId());
  return it != pending_.end() && it->second.published;
}

CommandStatusReasonEnumType ConditionalAddProvider::isCommandFailed(const std::weak_ptr<CmdSession> session) {
  std::shared_ptr<CmdSession> cmdSession = session.lock();
  if (!cmdSession) {
    return CommandStatusReasonEnumType::SERVICE_FAILED;
  }
  auto it = pending_.find(cmdSession->getSessionId());
  return it == pending_.end() ? CommandStatusReasonEnumType::SUCCEEDED : it->second.failReason;
}

bool ConditionalAddProvider::onCompleted(const std::weak_ptr<CmdSession> session) {
  erasePending(session);
  return true;
}

bool ConditionalAddProvider::onCanceled(const std::weak_ptr<CmdSession> session) {
  erasePending(session);
  return true;
}

bool ConditionalAddProvider::onFailed(const std::weak_ptr<CmdSession> session) {
  erasePending(session);
  return true;
}

void ConditionalAddProvider::erasePending(const std::weak_ptr<CmdSession> session) {
  if (std::shared_ptr<CmdSession> cmdSession = session.lock()) {
    pending_.erase(cmdSession->getSessionId());
  }
}

CommandStateResult ConditionalAddProvider::dispatchResolve(const ConditionalType& requested) {
  const std::string& topic = requested.specializationTopic();

  if (topic == UMAA::MM::Conditional::ConstraintViolatedConditionalTypeTopic) {
    return resolveAndPublish(&factoryIo_->ConstraintViolatedCache, requested);
  } else if (topic == UMAA::MM::Conditional::DepthConditionalTypeTopic) {
    return resolveAndPublish(&factoryIo_->DepthCache, requested);
  } else if (topic == UMAA::MM::Conditional::DepthRateConditionalTypeTopic) {
    return resolveAndPublish(&factoryIo_->DepthRateCache, requested);
  } else if (topic == UMAA::MM::Conditional::EmitterPresetConditionalTypeTopic) {
    return resolveAndPublish(&factoryIo_->EmitterPresetCache, requested);
  } else if (topic == UMAA::MM::Conditional::ExpConditionalTypeTopic) {
    return resolveAndPublish(&factoryIo_->ExpCache, requested);
  } else if (topic == UMAA::MM::Conditional::HeadingSectorConditionalTypeTopic) {
    return resolveAndPublish(&factoryIo_->HeadingSectorCache, requested);
  } else if (topic == UMAA::MM::Conditional::LogicalANDConditionalTypeTopic) {
    return resolveAndPublish(&factoryIo_->LogicalANDCache, requested);
  } else if (topic == UMAA::MM::Conditional::LogicalNOTConditionalTypeTopic) {
    return resolveAndPublish(&factoryIo_->LogicalNOTCache, requested);
  } else if (topic == UMAA::MM::Conditional::LogicalORConditionalTypeTopic) {
    return resolveAndPublish(&factoryIo_->LogicalORCache, requested);
  } else if (topic == UMAA::MM::Conditional::MissionStateConditionalTypeTopic) {
    return resolveAndPublish(&factoryIo_->MissionStateCache, requested);
  } else if (topic == UMAA::MM::Conditional::ObjectiveStateConditionalTypeTopic) {
    return resolveAndPublish(&factoryIo_->ObjectiveStateCache, requested);
  } else if (topic == UMAA::MM::Conditional::PitchRateConditionalTypeTopic) {
    return resolveAndPublish(&factoryIo_->PitchRateCache, requested);
  } else if (topic == UMAA::MM::Conditional::RelativeSpeedConditionalTypeTopic) {
    return resolveAndPublish(&factoryIo_->RelativeSpeedCache, requested);
  } else if (topic == UMAA::MM::Conditional::RollRateConditionalTypeTopic) {
    return resolveAndPublish(&factoryIo_->RollRateCache, requested);
  } else if (topic == UMAA::MM::Conditional::SpeedConditionalTypeTopic) {
    return resolveAndPublish(&factoryIo_->SpeedCache, requested);
  } else if (topic == UMAA::MM::Conditional::TaskStateConditionalTypeTopic) {
    return resolveAndPublish(&factoryIo_->TaskStateCache, requested);
  } else if (topic == UMAA::MM::Conditional::TimeConditionalTypeTopic) {
    return resolveAndPublish(&factoryIo_->TimeCache, requested);
  } else if (topic == UMAA::MM::Conditional::WaterZoneConditionalTypeTopic) {
    return resolveAndPublish(&factoryIo_->WaterZoneCache, requested);
  } else if (topic == UMAA::MM::Conditional::YawRateConditionalTypeTopic) {
    return resolveAndPublish(&factoryIo_->YawRateCache, requested);
  }

  UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Unknown or unsupported specialization topic: " << topic)
  return CommandStateResult::ERROR;
}

template <class Specialized>
CommandStateResult ConditionalAddProvider::resolveAndPublish(SpecializationCache<Specialized>* cache,
    const ConditionalType& requested) {
  std::optional<Specialized> payload = cache->getSpecialization(requested);
  if (!payload.has_value()) {
    return CommandStateResult::OK;
  }

  Specialized specialization = payload.value();
  auto [topic, writer] = reportProvider_->getTopicAndWriter(specialization);
  if (topic == "INVALID" || !writer) {
    UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "No report writer available for specialization topic: "
      << requested.specializationTopic())
    return CommandStateResult::ERROR;
  }

  // The commander's payload instance dies with its writer; take ownership by re-publishing under the report
  // provider's writer with a fresh specializationReferenceID. The commander-minted conditionalID is preserved
  // as the upsert key.
  specialization.specializationReferenceID(UuidFactory::getInstance().generateGuid());

  ConditionalType generic = requested;
  UMAA::Common::Measurement::DateTime sync = getTimestamp();
  specialization.specializationReferenceTimestamp(sync);
  generic.specializationID(specialization.specializationReferenceID());
  generic.specializationTimestamp(sync);

  std::optional<ConditionalType> existing = reportProvider_->getConditionalById(
    NumericGuid(requested.conditionalID()));

  if (writer->send(specialization) != SendStatus::SUCCESS) {
    UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Failed to publish specialization payload for conditional " <<
      NumericGuid(requested.conditionalID()))
    return CommandStateResult::ERROR;
  }

  SendStatus result;
  if (existing.has_value()) {
    if (reportProvider_->disposeConditionalSpecialization(existing.value()) != SendStatus::SUCCESS) {
      UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Failed to dispose superseded specialization while upserting conditional "
        << NumericGuid(requested.conditionalID()))
    }
    result = reportProvider_->updateConditional(generic);
  } else {
    result = reportProvider_->addConditional(generic);
  }

  if (result != SendStatus::SUCCESS) {
    UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Failed to insert conditional " << NumericGuid(requested.conditionalID())
      << " into the working report")
    return CommandStateResult::ERROR;
  }

  if (reportProvider_->sendReport() != SendStatus::SUCCESS) {
    UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Failed to send conditional report after adding conditional " <<
      NumericGuid(requested.conditionalID()))
    return CommandStateResult::ERROR;
  }

  return CommandStateResult::ADVANCE;
}

}  // namespace arlcore::umaa::conditional
