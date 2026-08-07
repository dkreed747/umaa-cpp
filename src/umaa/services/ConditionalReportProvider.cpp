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

#include "ConditionalReportProvider.h"

using UMAA::MM::ConditionalReport::ConditionalReportType;
using UMAA::MM::ConditionalReport::ConditionalReportTypeConditionalsSetElement;
using UMAA::MM::Conditional::ConditionalType;
using UMAA::MM::Conditional::ConstraintViolatedConditionalType;
using UMAA::MM::Conditional::ConstraintViolatedConditionalTypeTopic;
using UMAA::MM::Conditional::DepthConditionalType;
using UMAA::MM::Conditional::DepthConditionalTypeTopic;
using UMAA::MM::Conditional::DepthRateConditionalType;
using UMAA::MM::Conditional::DepthRateConditionalTypeTopic;
using UMAA::MM::Conditional::EmitterPresetConditionalType;
using UMAA::MM::Conditional::EmitterPresetConditionalTypeTopic;
using UMAA::MM::Conditional::ExpConditionalType;
using UMAA::MM::Conditional::ExpConditionalTypeTopic;
using UMAA::MM::Conditional::HeadingSectorConditionalType;
using UMAA::MM::Conditional::HeadingSectorConditionalTypeTopic;
using UMAA::MM::Conditional::LogicalANDConditionalType;
using UMAA::MM::Conditional::LogicalANDConditionalTypeTopic;
using UMAA::MM::Conditional::LogicalNOTConditionalType;
using UMAA::MM::Conditional::LogicalNOTConditionalTypeTopic;
using UMAA::MM::Conditional::LogicalORConditionalType;
using UMAA::MM::Conditional::LogicalORConditionalTypeTopic;
using UMAA::MM::Conditional::MissionStateConditionalType;
using UMAA::MM::Conditional::MissionStateConditionalTypeTopic;
using UMAA::MM::Conditional::ObjectiveStateConditionalType;
using UMAA::MM::Conditional::ObjectiveStateConditionalTypeTopic;
using UMAA::MM::Conditional::PitchRateConditionalType;
using UMAA::MM::Conditional::PitchRateConditionalTypeTopic;
using UMAA::MM::Conditional::RelativeSpeedConditionalType;
using UMAA::MM::Conditional::RelativeSpeedConditionalTypeTopic;
using UMAA::MM::Conditional::RollRateConditionalType;
using UMAA::MM::Conditional::RollRateConditionalTypeTopic;
using UMAA::MM::Conditional::SpeedConditionalType;
using UMAA::MM::Conditional::SpeedConditionalTypeTopic;
using UMAA::MM::Conditional::TaskStateConditionalType;
using UMAA::MM::Conditional::TaskStateConditionalTypeTopic;
using UMAA::MM::Conditional::TimeConditionalType;
using UMAA::MM::Conditional::TimeConditionalTypeTopic;
using UMAA::MM::Conditional::WaterZoneConditionalType;
using UMAA::MM::Conditional::WaterZoneConditionalTypeTopic;
using UMAA::MM::Conditional::YawRateConditionalType;
using UMAA::MM::Conditional::YawRateConditionalTypeTopic;

namespace arlcore::umaa::conditional {

ConditionalReportProvider::ConditionalReportProvider(
  const NumericGuid& sourceId,
  std::shared_ptr<ConditionalReportProviderIo> io,
  const NumericGuid& parentId)
  : sourceId_(sourceId),
  ConditionalReportProviderBase(sourceId, io->ReportWriter, parentId),
  ConditionalLargeSetWriterBase(io->ReportSetElementWriter),
  io_(io) {
}

SendStatus ConditionalReportProvider::sendReport() {
  ConditionalReportType report;
  report.conditionalsSetMetadata(this->getMetadata());
  return this->send(&report);
}

std::vector<ConditionalType> ConditionalReportProvider::getConditionals() const {
  std::vector<ConditionalType> conditionals;

  // transform set of SetElements to vector of their ConditionalType contents
  std::transform(this->begin(), this->end(), std::back_inserter(conditionals),
    [](auto element) { return element.element(); });
  return conditionals;
}

std::optional<ConditionalType> ConditionalReportProvider::getConditionalById(const NumericGuid &id) const {
  for (auto it = this->begin(); it != this->end(); ++it) {
    if (id == it->element().conditionalID()) {
      return it->element();
    }
  }
  return std::nullopt;
}

SendStatus ConditionalReportProvider::disposeConditionalSpecialization(ConditionalType cond) {
  if (cond.specializationTopic() == ConstraintViolatedConditionalTypeTopic) {
    ConstraintViolatedConditionalType c;
    c.specializationReferenceID(cond.specializationID());
    return io_->ConstraintViolatedWriter->dispose(c);
  } else if (cond.specializationTopic() == DepthConditionalTypeTopic) {
    DepthConditionalType c;
    c.specializationReferenceID(cond.specializationID());
    return io_->DepthWriter->dispose(c);
  } else if (cond.specializationTopic() == DepthRateConditionalTypeTopic) {
    DepthRateConditionalType c;
    c.specializationReferenceID(cond.specializationID());
    return io_->DepthRateWriter->dispose(c);
  } else if (cond.specializationTopic() == EmitterPresetConditionalTypeTopic) {
    EmitterPresetConditionalType c;
    c.specializationReferenceID(cond.specializationID());
    return io_->EmitterPresetWriter->dispose(c);
  } else if (cond.specializationTopic() == ExpConditionalTypeTopic) {
    ExpConditionalType c;
    c.specializationReferenceID(cond.specializationID());
    return io_->ExpWriter->dispose(c);
  } else if (cond.specializationTopic() == HeadingSectorConditionalTypeTopic) {
    HeadingSectorConditionalType c;
    c.specializationReferenceID(cond.specializationID());
    return io_->HeadingSectorWriter->dispose(c);
  } else if (cond.specializationTopic() == LogicalANDConditionalTypeTopic) {
    LogicalANDConditionalType c;
    c.specializationReferenceID(cond.specializationID());
    return io_->LogicalANDWriter->dispose(c);
  } else if (cond.specializationTopic() == LogicalNOTConditionalTypeTopic) {
    LogicalNOTConditionalType c;
    c.specializationReferenceID(cond.specializationID());
    return io_->LogicalNOTWriter->dispose(c);
  } else if (cond.specializationTopic() == LogicalORConditionalTypeTopic) {
    LogicalORConditionalType c;
    c.specializationReferenceID(cond.specializationID());
    return io_->LogicalORWriter->dispose(c);
  } else if (cond.specializationTopic() == MissionStateConditionalTypeTopic) {
    MissionStateConditionalType c;
    c.specializationReferenceID(cond.specializationID());
    return io_->MissionStateWriter->dispose(c);
  } else if (cond.specializationTopic() == ObjectiveStateConditionalTypeTopic) {
    ObjectiveStateConditionalType c;
    c.specializationReferenceID(cond.specializationID());
    return io_->ObjectiveStateWriter->dispose(c);
  } else if (cond.specializationTopic() == PitchRateConditionalTypeTopic) {
    PitchRateConditionalType c;
    c.specializationReferenceID(cond.specializationID());
    return io_->PitchRateWriter->dispose(c);
  } else if (cond.specializationTopic() == RelativeSpeedConditionalTypeTopic) {
    RelativeSpeedConditionalType c;
    c.specializationReferenceID(cond.specializationID());
    return io_->RelativeSpeedWriter->dispose(c);
  } else if (cond.specializationTopic() == RollRateConditionalTypeTopic) {
    RollRateConditionalType c;
    c.specializationReferenceID(cond.specializationID());
    return io_->RollRateWriter->dispose(c);
  } else if (cond.specializationTopic() == SpeedConditionalTypeTopic) {
    SpeedConditionalType c;
    c.specializationReferenceID(cond.specializationID());
    return io_->SpeedWriter->dispose(c);
  } else if (cond.specializationTopic() == TaskStateConditionalTypeTopic) {
    TaskStateConditionalType c;
    c.specializationReferenceID(cond.specializationID());
    return io_->TaskStateWriter->dispose(c);
  } else if (cond.specializationTopic() == TimeConditionalTypeTopic) {
    TimeConditionalType c;
    c.specializationReferenceID(cond.specializationID());
    return io_->TimeWriter->dispose(c);
  } else if (cond.specializationTopic() == WaterZoneConditionalTypeTopic) {
    WaterZoneConditionalType c;
    c.specializationReferenceID(cond.specializationID());
    return io_->WaterZoneWriter->dispose(c);
  } else if (cond.specializationTopic() == YawRateConditionalTypeTopic) {
    YawRateConditionalType c;
    c.specializationReferenceID(cond.specializationID());
    return io_->YawRateWriter->dispose(c);
  } else {
    UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Unknown or unsupported Specialization Topic: " <<
      cond.specializationTopic())
    return SendStatus::NOT_IMPLEMENTED;
  }
}

SendStatus ConditionalReportProvider::addConditional(const ConditionalType &conditional) {
  return this->insert(conditional);
}

SendStatus ConditionalReportProvider::updateConditional(const ConditionalType &conditional) {
  std::optional<ConditionalType> prev = getConditionalById(NumericGuid(conditional.conditionalID()));
  if (!prev.has_value()) {
    UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Attempted to update a conditional with unknown ID: " <<
      NumericGuid(conditional.conditionalID()))
    return SendStatus::ERROR;
  }
  return this->update(prev.value(), conditional);
}

SendStatus ConditionalReportProvider::removeConditional(const ConditionalType &conditional) {
  return this->remove(conditional);
}

SendStatus ConditionalReportProvider::removeSpecialization(const NumericGuid &conditionalId) {
  std::optional<ConditionalType> existing = getConditionalById(conditionalId);
  if (!existing.has_value()) {
    UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Attempted to remove conditional with unknown ID: " << conditionalId)
    return SendStatus::ERROR;
  }
  if (disposeConditionalSpecialization(existing.value()) != SendStatus::SUCCESS) {
    UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Unable to dispose specialized instance with specializationID: " <<
      NumericGuid(existing->specializationID()))
  }
  return this->remove(existing.value());
}

template <>
std::pair<std::string, std::shared_ptr<arlcore::io::SenderBase<ConstraintViolatedConditionalType>>>
  ConditionalReportProvider::getTopicAndWriter(ConstraintViolatedConditionalType cond) {
    return std::make_pair(UMAA::MM::Conditional::ConstraintViolatedConditionalTypeTopic, io_->ConstraintViolatedWriter);
}

template <>
std::pair<std::string, std::shared_ptr<arlcore::io::SenderBase<DepthConditionalType>>>
  ConditionalReportProvider::getTopicAndWriter(DepthConditionalType cond) {
    return std::make_pair(UMAA::MM::Conditional::DepthConditionalTypeTopic, io_->DepthWriter);
}

template <>
std::pair<std::string, std::shared_ptr<arlcore::io::SenderBase<DepthRateConditionalType>>>
  ConditionalReportProvider::getTopicAndWriter(DepthRateConditionalType cond) {
    return std::make_pair(UMAA::MM::Conditional::DepthRateConditionalTypeTopic, io_->DepthRateWriter);
}

template <>
std::pair<std::string, std::shared_ptr<arlcore::io::SenderBase<EmitterPresetConditionalType>>>
  ConditionalReportProvider::getTopicAndWriter(EmitterPresetConditionalType cond) {
    return std::make_pair(UMAA::MM::Conditional::EmitterPresetConditionalTypeTopic, io_->EmitterPresetWriter);
}

template <>
std::pair<std::string, std::shared_ptr<arlcore::io::SenderBase<ExpConditionalType>>>
  ConditionalReportProvider::getTopicAndWriter(ExpConditionalType cond) {
    return std::make_pair(UMAA::MM::Conditional::ExpConditionalTypeTopic, io_->ExpWriter);
}

template <>
std::pair<std::string, std::shared_ptr<arlcore::io::SenderBase<HeadingSectorConditionalType>>>
  ConditionalReportProvider::getTopicAndWriter(HeadingSectorConditionalType cond) {
    return std::make_pair(UMAA::MM::Conditional::HeadingSectorConditionalTypeTopic, io_->HeadingSectorWriter);
}

template <>
std::pair<std::string, std::shared_ptr<arlcore::io::SenderBase<LogicalANDConditionalType>>>
  ConditionalReportProvider::getTopicAndWriter(LogicalANDConditionalType cond) {
    return std::make_pair(UMAA::MM::Conditional::LogicalANDConditionalTypeTopic, io_->LogicalANDWriter);
}

template <>
std::pair<std::string, std::shared_ptr<arlcore::io::SenderBase<LogicalNOTConditionalType>>>
  ConditionalReportProvider::getTopicAndWriter(LogicalNOTConditionalType cond) {
    return std::make_pair(UMAA::MM::Conditional::LogicalNOTConditionalTypeTopic, io_->LogicalNOTWriter);
}

template <>
std::pair<std::string, std::shared_ptr<arlcore::io::SenderBase<LogicalORConditionalType>>>
  ConditionalReportProvider::getTopicAndWriter(LogicalORConditionalType cond) {
    return std::make_pair(UMAA::MM::Conditional::LogicalORConditionalTypeTopic, io_->LogicalORWriter);
}

template <>
std::pair<std::string, std::shared_ptr<arlcore::io::SenderBase<MissionStateConditionalType>>>
  ConditionalReportProvider::getTopicAndWriter(MissionStateConditionalType cond) {
    return std::make_pair(UMAA::MM::Conditional::MissionStateConditionalTypeTopic, io_->MissionStateWriter);
}

template <>
std::pair<std::string, std::shared_ptr<arlcore::io::SenderBase<ObjectiveStateConditionalType>>>
  ConditionalReportProvider::getTopicAndWriter(ObjectiveStateConditionalType cond) {
    return std::make_pair(UMAA::MM::Conditional::ObjectiveStateConditionalTypeTopic, io_->ObjectiveStateWriter);
}

template <>
std::pair<std::string, std::shared_ptr<arlcore::io::SenderBase<PitchRateConditionalType>>>
  ConditionalReportProvider::getTopicAndWriter(PitchRateConditionalType cond) {
    return std::make_pair(UMAA::MM::Conditional::PitchRateConditionalTypeTopic, io_->PitchRateWriter);
}

template <>
std::pair<std::string, std::shared_ptr<arlcore::io::SenderBase<RelativeSpeedConditionalType>>>
  ConditionalReportProvider::getTopicAndWriter(RelativeSpeedConditionalType cond) {
    return std::make_pair(UMAA::MM::Conditional::RelativeSpeedConditionalTypeTopic, io_->RelativeSpeedWriter);
}

template <>
std::pair<std::string, std::shared_ptr<arlcore::io::SenderBase<RollRateConditionalType>>>
  ConditionalReportProvider::getTopicAndWriter(RollRateConditionalType cond) {
    return std::make_pair(UMAA::MM::Conditional::RollRateConditionalTypeTopic, io_->RollRateWriter);
}

template <>
std::pair<std::string, std::shared_ptr<arlcore::io::SenderBase<SpeedConditionalType>>>
  ConditionalReportProvider::getTopicAndWriter(SpeedConditionalType cond) {
    return std::make_pair(UMAA::MM::Conditional::SpeedConditionalTypeTopic, io_->SpeedWriter);
}

template <>
std::pair<std::string, std::shared_ptr<arlcore::io::SenderBase<TaskStateConditionalType>>>
  ConditionalReportProvider::getTopicAndWriter(TaskStateConditionalType cond) {
    return std::make_pair(UMAA::MM::Conditional::TaskStateConditionalTypeTopic, io_->TaskStateWriter);
}

template <>
std::pair<std::string, std::shared_ptr<arlcore::io::SenderBase<TimeConditionalType>>>
  ConditionalReportProvider::getTopicAndWriter(TimeConditionalType cond) {
    return std::make_pair(UMAA::MM::Conditional::TimeConditionalTypeTopic, io_->TimeWriter);
}

template <>
std::pair<std::string, std::shared_ptr<arlcore::io::SenderBase<WaterZoneConditionalType>>>
  ConditionalReportProvider::getTopicAndWriter(WaterZoneConditionalType cond) {
    return std::make_pair(UMAA::MM::Conditional::WaterZoneConditionalTypeTopic, io_->WaterZoneWriter);
}

template <>
std::pair<std::string, std::shared_ptr<arlcore::io::SenderBase<YawRateConditionalType>>>
  ConditionalReportProvider::getTopicAndWriter(YawRateConditionalType cond) {
    return std::make_pair(UMAA::MM::Conditional::YawRateConditionalTypeTopic, io_->YawRateWriter);
}

}  // namespace arlcore::umaa::conditional
