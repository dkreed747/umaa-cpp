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

#ifndef INCLUDE_UMAA_CONDITIONALS_DEPTHRATECONDITIONAL_H_
#define INCLUDE_UMAA_CONDITIONALS_DEPTHRATECONDITIONAL_H_

#include <UMAA/MM/Conditional/DepthRateConditionalType.hpp>
#include <UMAA/SA/VelocityStatus/VelocityReportType.hpp>

#include "ConditionalBase.h"
#include "DdsIoTypes.h"
#include "Observer.h"
#include "SystemClock.h"

namespace arlcore::umaa::conditional {

using UMAA::MM::Conditional::DepthRateConditionalType;
using UMAA::MM::Conditional::DepthRateConditionalTypeTopic;
using UMAA::SA::VelocityStatus::VelocityReportType;

using arlcore::Observer;
using arlcore::io::SampleEnvelope;

//! @brief This class defines a depth rate conditional. The conditional is true when the current down speed
//! (depthRate), provided in VelocityReportType, has the relationship specified in conditionalOp to the specified
//! depthRate.
class DepthRateConditional : public ConditionalBase, public Observer<VelocityReportType> {
 public:
  //! \brief Constructor
  //! \param conditional The base UMAA conditional type
  //! \param derivedConditional The specialized DepthRateConditional UMAA type
  DepthRateConditional(
    const ConditionalType& conditional,
    const DepthRateConditionalType& derivedConditional) :
    ConditionalBase(conditional),
    derivedConditional_(derivedConditional) {
    if (!isValidSpecialization(conditional, derivedConditional, DepthRateConditionalTypeTopic)) {
      throw std::invalid_argument("Conditional does not match specialization");
    }
  }

  //! \brief Get the conditional operator associated with this conditional
  //! \return The conditional operator
  ConditionalOperatorEnumType getConditionalOperatorEnum() const {
    return derivedConditional_.conditionalOp();
  }

  //! \brief Get the depth rate specified by the conditional to compare to the current depth rate
  //! \return The depth rate specified by the conditional
  flt64_t getConditionalDepthRate() const {
    return derivedConditional_.depthRate();
  }

  //! \brief Overridden function to evaluate whether the stored conditional is true or false
  //! \return The evaluation status of the conditional (boolean). If a failure to evaluate the conditional occurs,
  //! the function returns nullopt.
  std::optional<bool> evaluateConditional() const override {
    if (vData_.has_value()) {
      return compareWithConditionalOp(vData_.value().velocity().downSpeed(), getConditionalDepthRate(),
        getConditionalOperatorEnum());
    } else {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Failed to get current depth data")
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
  DepthRateConditionalType getSpecializedConditional() const {
    return derivedConditional_;
  }

 private:
  DepthRateConditionalType derivedConditional_;
  std::optional<VelocityReportType> vData_;
};

}  // namespace arlcore::umaa::conditional
#endif  // INCLUDE_UMAA_CONDITIONALS_DEPTHRATECONDITIONAL_H_


