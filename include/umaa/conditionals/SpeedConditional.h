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

#ifndef INCLUDE_UMAA_CONDITIONALS_SPEEDCONDITIONAL_H_
#define INCLUDE_UMAA_CONDITIONALS_SPEEDCONDITIONAL_H_

#include <UMAA/MM/Conditional/SpeedConditionalType.hpp>
#include <UMAA/SA/SpeedStatus/SpeedReportType.hpp>

#include "ConditionalBase.h"
#include "DdsIoTypes.h"
#include "Observer.h"

namespace arlcore::umaa::conditional {

using UMAA::MM::Conditional::SpeedConditionalType;
using UMAA::MM::Conditional::SpeedConditionalTypeTopic;
using UMAA::SA::SpeedStatus::SpeedReportType;

using arlcore::Observer;
using arlcore::io::SampleEnvelope;

//! @brief This class defines a Speed conditional. The conditional is true when the current speedOverGround,
//! provided in SpeedReportType, has the relationship specified in conditionalOp to the specified speed.
class SpeedConditional : public ConditionalBase, public Observer<SpeedReportType> {
 public:
  //! \brief Constructor
  //! \param conditional The base UMAA conditional type
  //! \param derivedConditional The specialized SpeedConditional UMAA type
  SpeedConditional(
    const ConditionalType& conditional,
    const SpeedConditionalType& derivedConditional) :
    ConditionalBase(conditional),
    derivedConditional_(derivedConditional) {
    if (!isValidSpecialization(conditional, derivedConditional, SpeedConditionalTypeTopic)) {
      throw std::invalid_argument("Conditional does not match specialization");
    }
  }

  //! \brief Get the conditional operator associated with this conditional
  //! \return The conditional operator
  ConditionalOperatorEnumType getConditionalOperatorEnum() const {
    return derivedConditional_.conditionalOp();
  }

  //! \brief Get the speed specified by the conditional to compare to the current speed over ground
  //! \return The speed specified by the conditional
  flt64_t getConditionalSpeed() const {
    return derivedConditional_.speed();
  }

  //! \brief Overridden function to evaluate whether the stored conditional is true or false
  //! \return The evaluation status of the conditional (boolean). If a failure to evaluate the conditional occurs,
  //! the function returns nullopt.
  std::optional<bool> evaluateConditional() const override {
    if (ssData_.has_value() && ssData_->speedOverGround().has_value()) {
      return compareWithConditionalOp(ssData_->speedOverGround().value(), getConditionalSpeed(),
        getConditionalOperatorEnum());
    } else {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Failed to get current speed data")
      return std::nullopt;
    }
  }

  //! \brief Overridden function that allows the conditional to act as an observer for speed report data
  //! \param ssData Updated speed report data
  void update(const SpeedReportType &ssData) override {
    ssData_ = ssData;
  }

  //! \brief Get the specialized UMAA conditional object that was used to initialize this object
  //! \return The UMAA specialized conditional object
  SpeedConditionalType getSpecializedConditional() const {
    return derivedConditional_;
  }

 private:
  SpeedConditionalType derivedConditional_;
  std::optional<SpeedReportType> ssData_;
};

}  // namespace arlcore::umaa::conditional
#endif  // INCLUDE_UMAA_CONDITIONALS_SPEEDCONDITIONAL_H_
