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

#ifndef INCLUDE_UMAA_CONDITIONALS_DEPTHCONDITIONAL_H_
#define INCLUDE_UMAA_CONDITIONALS_DEPTHCONDITIONAL_H_

#include <functional>
#include <memory>
#include <optional>

#include <UMAA/MM/Conditional/DepthConditionalType.hpp>
#include <UMAA/SA/GlobalPoseStatus/GlobalPoseReportType.hpp>

#include "ConditionalBase.h"
#include "DdsIoTypes.h"
#include "Observer.h"

namespace arlcore::umaa::conditional {

using UMAA::MM::Conditional::DepthConditionalType;
using UMAA::MM::Conditional::DepthConditionalTypeTopic;
using UMAA::SA::GlobalPoseStatus::GlobalPoseReportType;

using arlcore::Observer;
using arlcore::io::SampleEnvelope;

//! \brief This class defines a depth conditional. The conditional is true when the current depth, provided
//! in GlobalPoseReportType, has the relationship specified in conditionalOp to the specified depth.
class DepthConditional : public ConditionalBase, public Observer<GlobalPoseReportType> {
 public:
  //! \brief Constructor
  //! \param conditional The base UMAA conditional type
  //! \param derivedConditional The specialized DepthConditional UMAA type
  DepthConditional(
    const ConditionalType& conditional,
    const DepthConditionalType& derivedConditional) :
    ConditionalBase(conditional),
    derivedConditional_(derivedConditional) {
    if (!isValidSpecialization(conditional, derivedConditional, DepthConditionalTypeTopic)) {
      throw std::invalid_argument("Conditional does not match specialization");
    }
  }

  //! \brief Get the conditional operator associated with this conditional
  //! \return The conditional operator
  ConditionalOperatorEnumType getConditionalOperatorEnum() const {
    return derivedConditional_.conditionalOp();
  }

  //! \brief Get the depth specified by the conditional to compare to the current depth
  //! \return The depth specified by the conditional
  flt64_t getConditionalDepth() const {
    return derivedConditional_.depth();
  }

  //! \brief Overridden function to evaluate whether the stored conditional is true or false
  //! \return The evaluation status of the conditional (boolean). If a failure to evaluate the conditional occurs,
  //! the function returns nullopt.
  std::optional<bool> evaluateConditional() const override {
    if (gpData_.has_value() && gpData_->depth().has_value()) {
      return compareWithConditionalOp(gpData_->depth().value(), getConditionalDepth(), getConditionalOperatorEnum());
    } else {
      UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Failed to get current depth data")
      return std::nullopt;
    }
  }

  //! \brief Overridden function that allows the conditional to act as an observer for global pose data
  //! \param gpData Updated global pose data
  void update(const GlobalPoseReportType &gpData) override {
    gpData_ = gpData;
  }

  //! \brief Get the specialized UMAA conditional object that was used to initialize this object
  //! \return The UMAA specialized conditional object
  DepthConditionalType getSpecializedConditional() const {
    return derivedConditional_;
  }

 private:
  DepthConditionalType derivedConditional_;
  std::optional<GlobalPoseReportType> gpData_;
};

}  // namespace arlcore::umaa::conditional
#endif  // INCLUDE_UMAA_CONDITIONALS_DEPTHCONDITIONAL_H_
