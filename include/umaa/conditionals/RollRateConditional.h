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

#ifndef INCLUDE_UMAA_CONDITIONALS_ROLLRATECONDITIONAL_H_
#define INCLUDE_UMAA_CONDITIONALS_ROLLRATECONDITIONAL_H_

#include <UMAA/MM/Conditional/RollRateConditionalType.hpp>
#include <UMAA/SA/VelocityStatus/VelocityReportType.hpp>

#include "ConditionalBase.h"
#include "DdsIoTypes.h"
#include "Observer.h"

namespace arlcore::umaa::conditional {

using UMAA::MM::Conditional::RollRateConditionalType;
using UMAA::MM::Conditional::RollRateConditionalTypeTopic;
using UMAA::SA::VelocityStatus::VelocityReportType;

using arlcore::Observer;
using arlcore::io::SampleEnvelope;

//! @brief This class defines a roll rate conditional. The conditional is true when the current rollRate, provided in
//! VelocityReportType, has the relationship specified in conditionalOp to the specified rollRate.
class RollRateConditional : public ConditionalBase, public Observer<VelocityReportType> {
 public:
  //! \brief Constructor
  //! \param conditional The base UMAA conditional type
  //! \param derivedConditional The specialized RollRateConditional UMAA type
  RollRateConditional(
    const ConditionalType& conditional,
    const RollRateConditionalType& derivedConditional) :
    ConditionalBase(conditional),
    derivedConditional_(derivedConditional) {
    if (!isValidSpecialization(conditional, derivedConditional, RollRateConditionalTypeTopic)) {
      throw std::invalid_argument("Conditional does not match specialization");
    }
  }

  //! \brief Get the conditional operator associated with this conditional
  //! \return The conditional operator
  ConditionalOperatorEnumType getConditionalOperatorEnum() const {
    return derivedConditional_.conditionalOp();
  }

  //! \brief Get the roll rate specified by the conditional to compare to the current roll rate
  //! \return The roll rate specified by the conditional
  flt64_t getConditionalRollRate() const {
    return derivedConditional_.rollRate();
  }

  //! \brief Overridden function to evaluate whether the stored conditional is true or false
  //! \return The evaluation status of the conditional (boolean). If a failure to evaluate the conditional occurs,
  //! the function returns nullopt.
  std::optional<bool> evaluateConditional() const override {
    if (vData_.has_value()) {
      return compareWithConditionalOp(vData_->attitudeRate().rollRate(), getConditionalRollRate(),
        getConditionalOperatorEnum());
    } else {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Failed to get current roll rate data")
      return std::nullopt;
    }
  }

  //! \brief Overridden function that allows the conditional to act as an observer for velocity report data
  //! \param vData Updated velocity report data
  void update(const VelocityReportType &vData) override {
    vData_ = vData;
  }

  //! \brief Get the specialized UMAA conditional object that was used to initialize this object
  //! \return The UMAA specialized conditional object
  RollRateConditionalType getSpecializedConditional() const {
    return derivedConditional_;
  }

 private:
  RollRateConditionalType derivedConditional_;
  std::optional<VelocityReportType> vData_;
};

}  // namespace arlcore::umaa::conditional
#endif  // INCLUDE_UMAA_CONDITIONALS_ROLLRATECONDITIONAL_H_
