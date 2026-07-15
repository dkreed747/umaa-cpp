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

#include "ConditionalReportConsumer.h"
#include "ConstraintViolatedConditional.h"
#include "DepthConditional.h"
#include "DepthRateConditional.h"
#include "HeadingSectorConditional.h"
#include "LogicalANDConditional.h"
#include "LogicalNOTConditional.h"
#include "LogicalORConditional.h"
#include "PitchRateConditional.h"
#include "RelativeSpeedConditional.h"
#include "RollRateConditional.h"
#include "SpeedConditional.h"
#include "TimeConditional.h"
#include "WaterZoneConditional.h"
#include "YawRateConditional.h"

namespace arlcore::umaa::conditional {

ConditionalReportConsumer::ConditionalReportConsumer(
    std::shared_ptr<arlcore::io::ReaderBase<ConditionalReportType>> reportReader,
    std::shared_ptr<arlcore::io::ReaderBase<ConditionalReportTypeConditionalsSetElement>> reportSetElementReader,
    std::shared_ptr<ConditionalFactory> factory)
    : ConditionalReportConsumerBase(reportReader),
    ConditionalLargeSetReaderBase(reportSetElementReader),
    factory_(factory) {
}

bool ConditionalReportConsumer::cycle() {
  if (ReadStatus status = ReportConsumer::cycle(); status == ReadStatus::ERROR) {
    return false;
  } else if (status != ReadStatus::SUCCESS) {
    return true;
  }

  auto conditionalSet = this->getSetFromMetadata(getReport()->conditionalsSetMetadata());
  if (conditionalSet.status == arlcore::umaa::LargeSetStatus::EMPTY_SET) {
    // An empty set is still a set change (e.g. every conditional deleted): observers such as
    // active-constraints providers must hear about it or they would keep evaluating
    // conditionals that no longer exist. Note EMPTY_SET carries a null set pointer.
    conditionals_.reset();
    conditionalObjects_ = std::vector<std::shared_ptr<ConditionalBase>>();
    notify(conditionalObjects_.value());
    return true;
  }
  if (conditionalSet.status == arlcore::umaa::LargeSetStatus::INVALID_SET || conditionalSet.set.expired()) {
    return false;
  }

  if (auto conditionals = conditionalSet.set.lock()) {
    if (conditionals->empty()) {
      conditionals_.reset();
      conditionalObjects_ = std::vector<std::shared_ptr<ConditionalBase>>();
      notify(conditionalObjects_.value());
      return true;
    }

    std::vector<ConditionalType> conditionalList;
    for (auto it = conditionals->begin(); it != conditionals->end(); ++it) {
      conditionalList.emplace_back(*it);
    }
    conditionals_ = conditionalList;
    conditionalObjects_ = factory_->createConditionals(conditionalList);

    if (conditionalObjects_.has_value()) {
      notify(conditionalObjects_.value());
    }
    return true;
  } else {
    return false;
  }
}

std::optional<std::vector<std::shared_ptr<ConditionalBase>>> ConditionalReportConsumer::getConditionals() {
  cycle();
  return conditionalObjects_;
}

}  // namespace arlcore::umaa::conditional
