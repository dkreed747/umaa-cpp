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

#ifndef INCLUDE_UMAA_CONDITIONALS_YAWRATECONDITIONAL_H_
#define INCLUDE_UMAA_CONDITIONALS_YAWRATECONDITIONAL_H_

#include <UMAA/MM/Conditional/YawRateConditionalType.hpp>
#include <UMAA/SA/VelocityStatus/VelocityReportType.hpp>

#include "ConditionalBase.h"
#include "DdsIoTypes.h"
#include "Observer.h"

namespace arlcore::umaa::conditional {

using UMAA::MM::Conditional::YawRateConditionalType;
using UMAA::MM::Conditional::YawRateConditionalTypeTopic;
using UMAA::SA::VelocityStatus::VelocityReportType;

using arlcore::Observer;
using arlcore::io::SampleEnvelope;

//! @brief This class defines a yaw rate conditional. The conditional is true when the current yawRate, provided in
//! VelocityReportType, has the relationship specified in conditionalOp to the specified yawRate.
class YawRateConditional : public ConditionalBase, public Observer<VelocityReportType> {
 public:
  //! \brief Constructor
  //! \param conditional The base UMAA conditional type
  //! \param derivedConditional The specialized YawRateConditional UMAA type
  YawRateConditional(
    const ConditionalType& conditional,
    const YawRateConditionalType& derivedConditional) :
    ConditionalBase(conditional),
    derivedConditional_(derivedConditional) {
    if (!isValidSpecialization(conditional, derivedConditional, YawRateConditionalTypeTopic)) {
      throw std::invalid_argument("Conditional does not match specialization");
    }
  }

  //! \brief Get the conditional operator associated with this conditional
  //! \return The conditional operator
  ConditionalOperatorEnumType getConditionalOperatorEnum() const {
    return derivedConditional_.conditionalOp();
  }

  //! \brief Get the yaw rate specified by the conditional to compare to the current yaw rate
  //! \return The yaw rate specified by the conditional
  flt64_t getConditionalYawRate() const {
    return derivedConditional_.yawRate();
  }

  //! \brief Overridden function to evaluate whether the stored conditional is true or false
  //! \return The evaluation status of the conditional (boolean). If a failure to evaluate the conditional occurs,
  //! the function returns nullopt.
  std::optional<bool> evaluateConditional() const override {
    if (vData_.has_value()) {
      return compareWithConditionalOp(vData_->attitudeRate().yawRate(), getConditionalYawRate(),
        getConditionalOperatorEnum());
    } else {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Failed to get current yaw rate from velocity report")
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
  YawRateConditionalType getSpecializedConditional() const {
    return derivedConditional_;
  }

 private:
  YawRateConditionalType derivedConditional_;
  std::optional<VelocityReportType> vData_;
};

}  // namespace arlcore::umaa::conditional
#endif  // INCLUDE_UMAA_CONDITIONALS_YAWRATECONDITIONAL_H_
