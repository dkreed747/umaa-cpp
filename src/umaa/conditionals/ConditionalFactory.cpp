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
#include "ConditionalFactory.h"

namespace arlcore::umaa::conditional {

bool ConditionalFactory::hasDependencies(const ConditionalType& conditional) {
  if (conditional.specializationTopic() == ConstraintViolatedConditionalTypeTopic ||
      conditional.specializationTopic() == LogicalANDConditionalTypeTopic ||
      conditional.specializationTopic() == LogicalNOTConditionalTypeTopic ||
      conditional.specializationTopic() == LogicalORConditionalTypeTopic) {
    return true;
  }
  return false;
}

std::optional<std::vector<std::shared_ptr<ConditionalBase>>> ConditionalFactory::createConditionals(
    std::vector<ConditionalType> generalizedConditionals) {
  auto conditionals = std::make_shared<std::map<NumericGUID, std::shared_ptr<ConditionalBase>>>();
  std::vector<NumericGUID> deferred;

  // Create objects for conditionals
  for (auto it = generalizedConditionals.begin(); it != generalizedConditionals.end(); ++it) {
    if (hasDependencies(*it)) {
      deferred.push_back(it->conditionalID());
    }
    if (it->specializationTopic() == ConstraintViolatedConditionalTypeTopic) {
      auto conditional = createConstraintViolatedConditional(*it);
      if (!conditional.has_value()) {
        return std::nullopt;
      }
      conditionals->insert(std::pair(it->conditionalID(),
                    std::static_pointer_cast<ConditionalBase, ConstraintViolatedConditional>(conditional.value())));
    } else if (it->specializationTopic() == DepthConditionalTypeTopic) {
      auto conditional = createDepthConditional(*it);
      if (!conditional.has_value()) {
        return std::nullopt;
      }
      io_->globalPoseReportConsumer_->getReportSubject().registerObserver(conditional.value());
      conditionals->insert(std::pair(it->conditionalID(),
                    std::static_pointer_cast<ConditionalBase, DepthConditional>(conditional.value())));
    } else if (it->specializationTopic() == DepthRateConditionalTypeTopic) {
      auto conditional = createDepthRateConditional(*it);
      if (!conditional.has_value()) {
        return std::nullopt;
      }
      io_->velocityReportConsumer_->getReportSubject().registerObserver(conditional.value());
      conditionals->insert(std::pair(it->conditionalID(),
                    std::static_pointer_cast<ConditionalBase, DepthRateConditional>(conditional.value())));
    } else if (it->specializationTopic() == HeadingSectorConditionalTypeTopic) {
      auto conditional = createHeadingSectorConditional(*it);
      if (!conditional.has_value()) {
        return std::nullopt;
      }
      io_->globalPoseReportConsumer_->getReportSubject().registerObserver(conditional.value());
      conditionals->insert(std::pair(it->conditionalID(),
                    std::static_pointer_cast<ConditionalBase, HeadingSectorConditional>(conditional.value())));
    } else if (it->specializationTopic() == LogicalANDConditionalTypeTopic) {
      auto conditional = createLogicalANDConditional(*it);
      if (!conditional.has_value()) {
        return std::nullopt;
      }
      conditionals->insert(std::pair(it->conditionalID(),
                    std::static_pointer_cast<ConditionalBase, LogicalANDConditional>(conditional.value())));
    } else if (it->specializationTopic() == LogicalNOTConditionalTypeTopic) {
      auto conditional = createLogicalNOTConditional(*it);
      if (!conditional.has_value()) {
        return std::nullopt;
      }
      conditionals->insert(std::pair(it->conditionalID(),
                    std::static_pointer_cast<ConditionalBase, LogicalNOTConditional>(conditional.value())));
    } else if (it->specializationTopic() == LogicalORConditionalTypeTopic) {
      auto conditional = createLogicalORConditional(*it);
      if (!conditional.has_value()) {
        return std::nullopt;
      }
      conditionals->insert(std::pair(it->conditionalID(),
                    std::static_pointer_cast<ConditionalBase, LogicalORConditional>(conditional.value())));
    } else if (it->specializationTopic() == PitchRateConditionalTypeTopic) {
      auto conditional = createPitchRateConditional(*it);
      if (!conditional.has_value()) {
        return std::nullopt;
      }
      io_->velocityReportConsumer_->getReportSubject().registerObserver(conditional.value());
      conditionals->insert(std::pair(it->conditionalID(),
                    std::static_pointer_cast<ConditionalBase, PitchRateConditional>(conditional.value())));
    } else if (it->specializationTopic() == RelativeSpeedConditionalTypeTopic) {
      auto conditional = createRelativeSpeedConditional(*it);
      if (!conditional.has_value()) {
        return std::nullopt;
      }
      io_->speedReportConsumer_->getReportSubject().registerObserver(conditional.value());
      conditionals->insert(std::pair(it->conditionalID(),
                    std::static_pointer_cast<ConditionalBase, RelativeSpeedConditional>(conditional.value())));
    } else if (it->specializationTopic() == RollRateConditionalTypeTopic) {
      auto conditional = createRollRateConditional(*it);
      if (!conditional.has_value()) {
        return std::nullopt;
      }
      io_->velocityReportConsumer_->getReportSubject().registerObserver(conditional.value());
      conditionals->insert(std::pair(it->conditionalID(),
                    std::static_pointer_cast<ConditionalBase, RollRateConditional>(conditional.value())));
    } else if (it->specializationTopic() == SpeedConditionalTypeTopic) {
      auto conditional = createSpeedConditional(*it);
      if (!conditional.has_value()) {
        return std::nullopt;
      }
      io_->speedReportConsumer_->getReportSubject().registerObserver(conditional.value());
      conditionals->insert(std::pair(it->conditionalID(),
                    std::static_pointer_cast<ConditionalBase, SpeedConditional>(conditional.value())));
    } else if (it->specializationTopic() == TimeConditionalTypeTopic) {
      auto conditional = createTimeConditional(*it);
      if (!conditional.has_value()) {
        return std::nullopt;
      }
      conditionals->insert(std::pair(it->conditionalID(),
                    std::static_pointer_cast<ConditionalBase, TimeConditional>(conditional.value())));
    } else if (it->specializationTopic() == WaterZoneConditionalTypeTopic) {
      auto conditional = createWaterZoneConditional(*it);
      if (!conditional.has_value()) {
        return std::nullopt;
      }
      io_->globalPoseReportConsumer_->getReportSubject().registerObserver(conditional.value());
      conditionals->insert(std::pair(it->conditionalID(),
                    std::static_pointer_cast<ConditionalBase, WaterZoneConditional>(conditional.value())));
    } else if (it->specializationTopic() == YawRateConditionalTypeTopic) {
      auto conditional = createYawRateConditional(*it);
      if (!conditional.has_value()) {
        return std::nullopt;
      }
      io_->velocityReportConsumer_->getReportSubject().registerObserver(conditional.value());
      conditionals->insert(std::pair(it->conditionalID(),
                    std::static_pointer_cast<ConditionalBase, YawRateConditional>(conditional.value())));
    } else {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Unknown or unsupported Specialization Topic: " <<
        it->specializationTopic())
      return std::nullopt;
    }
  }

  // Add references to dependent conditionals
  for (auto it = deferred.begin(); it != deferred.end(); ++it) {
    auto conditional = conditionals->find(*it);
    if (conditional == conditionals->end()) {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Unable to find conditional object with dependencies")
      return std::nullopt;
    }
    if (conditional->second->getSpecializationTopic() == ConstraintViolatedConditionalTypeTopic) {
      auto c = ConditionalBase::getSpecialized<ConstraintViolatedConditional>(conditional->second);
      auto constraint = conditionals->find(c->getConstraintConditionalId().getGuid());
      if (constraint == conditionals->end()) {
        UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Unable to find conditional corresponding to constraint conditional")
        return std::nullopt;
      }
      c->setConstraintConditional(constraint->second);
    } else if (conditional->second->getSpecializationTopic() == LogicalANDConditionalTypeTopic) {
      auto c = ConditionalBase::getSpecialized<LogicalANDConditional>(conditional->second);
      auto [firstId, secondId] = c->getReferencedConditionalIds();
      auto first = conditionals->find(firstId.getGuid());
      auto second = conditionals->find(secondId.getGuid());
      if (first == conditionals->end() || second == conditionals->end()) {
        UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Unable to find conditional corresponding to logical and conditional")
        return std::nullopt;
      }
      c->setReferencedConditionals(first->second, second->second);
    } else if (conditional->second->getSpecializationTopic() == LogicalNOTConditionalTypeTopic) {
      auto c = ConditionalBase::getSpecialized<LogicalNOTConditional>(conditional->second);
      auto negation = conditionals->find(c->getNotConditionalId().getGuid());
      if (negation == conditionals->end()) {
        UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Unable to find conditional corresponding to logical not conditional")
        return std::nullopt;
      }
      c->setNegatedConditional(negation->second);
    } else if (conditional->second->getSpecializationTopic() == LogicalORConditionalTypeTopic) {
      auto c = ConditionalBase::getSpecialized<LogicalORConditional>(conditional->second);
      auto [firstId, secondId] = c->getReferencedConditionalIds();
      auto first = conditionals->find(firstId.getGuid());
      auto second = conditionals->find(secondId.getGuid());
      if (first == conditionals->end() || second == conditionals->end()) {
        UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Unable to find conditional corresponding to logical or conditional")
        return std::nullopt;
      }
      c->setReferencedConditionals(first->second, second->second);
    }
  }

  // Make sure there are no circular dependencies
  auto visited = std::make_shared<std::map<NumericGUID, bool>>();
  for (auto it = deferred.begin(); it != deferred.end(); ++it) {
    visited->clear();

    if (hasConflict(*it, conditionals)) {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Circular dependency detected in conditionals")
      return std::nullopt;
    }
  }

  // Convert the map to a vector
  std::vector<std::shared_ptr<ConditionalBase>> result;
  for (auto it = conditionals->begin(); it != conditionals->end(); ++it) {
    result.push_back(it->second);
  }
  return result;
}

bool ConditionalFactory::hasConflict(NumericGUID node, std::shared_ptr<std::map<NumericGUID,
                                     std::shared_ptr<ConditionalBase>>> nodes) {
  static std::map<NumericGUID, bool> lookup;
  if (auto res = lookup.find(node); res != lookup.end() && res->second == true) {
    lookup.erase(node);
    return true;
  }
  lookup.insert_or_assign(node, true);
  auto [firstId, secondId] = nodes->at(node)->getDependencies();
  bool first = false;
  bool second = false;
  if (firstId.has_value()) {
    first = hasConflict(firstId->getGuid(), nodes);
  }
  if (secondId.has_value()) {
    second = hasConflict(secondId->getGuid(), nodes);
  }
  lookup.erase(node);
  return first || second;
}

template <class Derived, class Specialization>
std::optional<std::shared_ptr<Derived>> ConditionalFactory::createConditional(const ConditionalType& base,
    const Specialization& specialized,
    const std::string& topic) {
  if (ConditionalBase::isValidSpecialization<Specialization>(base, specialized, topic)) {
    return std::make_shared<Derived>(base, specialized);
  }
  return std::nullopt;
}

std::optional<std::shared_ptr<ConstraintViolatedConditional>> ConditionalFactory::createConstraintViolatedConditional(
    const ConditionalType& base) {
  auto specialized = io_->ConstraintViolatedCache.getSpecialization<ConditionalType>(base);
  if (!specialized.has_value()) {
    return std::nullopt;
  }
  return createConditional<ConstraintViolatedConditional, ConstraintViolatedConditionalType>(base,
    specialized.value(), ConstraintViolatedConditionalTypeTopic);
}

std::optional<std::shared_ptr<DepthConditional>> ConditionalFactory::createDepthConditional(
  const ConditionalType& base) {
  auto specialized = io_->DepthCache.getSpecialization<ConditionalType>(base);
  if (!specialized.has_value()) {
    return std::nullopt;
  }
  return createConditional<DepthConditional, DepthConditionalType>(base, specialized.value(),
    DepthConditionalTypeTopic);
}

std::optional<std::shared_ptr<DepthRateConditional>> ConditionalFactory::createDepthRateConditional(
    const ConditionalType& base) {
  auto specialized = io_->DepthRateCache.getSpecialization<ConditionalType>(base);
  if (!specialized.has_value()) {
    return std::nullopt;
  }
  return createConditional<DepthRateConditional, DepthRateConditionalType>(base, specialized.value(),
    DepthRateConditionalTypeTopic);
}

std::optional<std::shared_ptr<HeadingSectorConditional>> ConditionalFactory::createHeadingSectorConditional(
    const ConditionalType& base) {
  auto specialized = io_->HeadingSectorCache.getSpecialization<ConditionalType>(base);
  if (!specialized.has_value()) {
    return std::nullopt;
  }
  return createConditional<HeadingSectorConditional, HeadingSectorConditionalType>(base, specialized.value(),
    HeadingSectorConditionalTypeTopic);
}

std::optional<std::shared_ptr<LogicalANDConditional>> ConditionalFactory::createLogicalANDConditional(
    const ConditionalType& base) {
  auto specialized = io_->LogicalANDCache.getSpecialization<ConditionalType>(base);
  if (!specialized.has_value()) {
    return std::nullopt;
  }
  return createConditional<LogicalANDConditional, LogicalANDConditionalType>(base, specialized.value(),
    LogicalANDConditionalTypeTopic);
}

std::optional<std::shared_ptr<LogicalNOTConditional>> ConditionalFactory::createLogicalNOTConditional(
    const ConditionalType& base) {
  auto specialized = io_->LogicalNOTCache.getSpecialization<ConditionalType>(base);
  if (!specialized.has_value()) {
    return std::nullopt;
  }
  return createConditional<LogicalNOTConditional, LogicalNOTConditionalType>(base, specialized.value(),
    LogicalNOTConditionalTypeTopic);
}

std::optional<std::shared_ptr<LogicalORConditional>> ConditionalFactory::createLogicalORConditional(
    const ConditionalType& base) {
  auto specialized = io_->LogicalORCache.getSpecialization<ConditionalType>(base);
  if (!specialized.has_value()) {
    return std::nullopt;
  }
  return createConditional<LogicalORConditional, LogicalORConditionalType>(base, specialized.value(),
    LogicalORConditionalTypeTopic);
}

std::optional<std::shared_ptr<PitchRateConditional>> ConditionalFactory::createPitchRateConditional(
    const ConditionalType& base) {
  auto specialized = io_->PitchRateCache.getSpecialization<ConditionalType>(base);
  if (!specialized.has_value()) {
    return std::nullopt;
  }
  return createConditional<PitchRateConditional, PitchRateConditionalType>(base, specialized.value(),
    PitchRateConditionalTypeTopic);
}

std::optional<std::shared_ptr<RelativeSpeedConditional>> ConditionalFactory::createRelativeSpeedConditional(
    const ConditionalType& base) {
  auto specialized = io_->RelativeSpeedCache.getSpecialization<ConditionalType>(base);
  if (!specialized.has_value()) {
    return std::nullopt;
  }
  return createConditional<RelativeSpeedConditional, RelativeSpeedConditionalType>(base, specialized.value(),
    RelativeSpeedConditionalTypeTopic);
}

std::optional<std::shared_ptr<RollRateConditional>> ConditionalFactory::createRollRateConditional(
    const ConditionalType& base) {
  auto specialized = io_->RollRateCache.getSpecialization<ConditionalType>(base);
  if (!specialized.has_value()) {
    return std::nullopt;
  }
  return createConditional<RollRateConditional, RollRateConditionalType>(base, specialized.value(),
    RollRateConditionalTypeTopic);
}

std::optional<std::shared_ptr<SpeedConditional>> ConditionalFactory::createSpeedConditional(
    const ConditionalType& base) {
  auto specialized = io_->SpeedCache.getSpecialization<ConditionalType>(base);
  if (!specialized.has_value()) {
    return std::nullopt;
  }
  return createConditional<SpeedConditional, SpeedConditionalType>(base, specialized.value(),
    SpeedConditionalTypeTopic);
}

std::optional<std::shared_ptr<TimeConditional>> ConditionalFactory::createTimeConditional(
    const ConditionalType& base) {
  auto specialized = io_->TimeCache.getSpecialization<ConditionalType>(base);
  if (!specialized.has_value()) {
    return std::nullopt;
  }
  return createConditional<TimeConditional, TimeConditionalType>(base, specialized.value(),
    TimeConditionalTypeTopic);
}

std::optional<std::shared_ptr<WaterZoneConditional>> ConditionalFactory::createWaterZoneConditional(
    const ConditionalType& base) {
  auto specialized = io_->WaterZoneCache.getSpecialization<ConditionalType>(base);
  if (!specialized.has_value()) {
    return std::nullopt;
  }
  return createConditional<WaterZoneConditional, WaterZoneConditionalType>(base, specialized.value(),
    WaterZoneConditionalTypeTopic);
}

std::optional<std::shared_ptr<YawRateConditional>> ConditionalFactory::createYawRateConditional(
    const ConditionalType& base) {
  auto specialized = io_->YawRateCache.getSpecialization<ConditionalType>(base);
  if (!specialized.has_value()) {
    return std::nullopt;
  }
  return createConditional<YawRateConditional, YawRateConditionalType>(base, specialized.value(),
    YawRateConditionalTypeTopic);
}

}  // namespace arlcore::umaa::conditional
