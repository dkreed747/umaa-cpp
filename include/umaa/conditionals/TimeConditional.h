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

#ifndef INCLUDE_UMAA_CONDITIONALS_TIMECONDITIONAL_H_
#define INCLUDE_UMAA_CONDITIONALS_TIMECONDITIONAL_H_

#include <UMAA/MM/Conditional/TimeConditionalType.hpp>

#include "ConditionalBase.h"
#include "RealtimeSystemClock.h"
#include "UmaaUtils.h"

namespace arlcore::umaa::conditional {

using UMAA::MM::Conditional::TimeConditionalType;
using UMAA::MM::Conditional::TimeConditionalTypeTopic;
using UMAA::Common::Measurement::DateTime;

//! @brief This class defines a time conditional. The conditional is true when the current time
//! has the relationship specified in conditionalOp to the specified time.
class TimeConditional : public ConditionalBase {
 public:
  //! \brief Constructor
  //! \param conditional The base UMAA conditional type
  //! \param derivedConditional The specialized TimeConditional UMAA type
  TimeConditional(
    const ConditionalType& conditional,
    const TimeConditionalType& derivedConditional) :
    ConditionalBase(conditional),
    derivedConditional_(derivedConditional) {
    if (!isValidSpecialization(conditional, derivedConditional, TimeConditionalTypeTopic)) {
      throw std::invalid_argument("Conditional does not match specialization");
    }
  }

  //! \brief Get the conditional operator associated with this conditional
  //! \return The conditional operator
  ConditionalOperatorEnumType getConditionalOperatorEnum() const {
    return derivedConditional_.conditionalOp();
  }

  //! \brief Get the time specified by the conditional to compare to the current time
  //! \return The time specified by the conditional
  DateTime getConditionalTime() const {
    return derivedConditional_.time();
  }

  //! \brief Overridden function to evaluate whether the stored conditional is true or false
  //! \return The evaluation status of the conditional (boolean). If a failure to evaluate the conditional occurs,
  //! the function returns nullopt.
  std::optional<bool> evaluateConditional() const override {
    static arlcore::RealtimeSystemClock clk;
    arlcore::timestamp current = clk.getCurrentTime_timestamp();
    return compareWithConditionalOp(DateTime(current.seconds, current.nanoseconds), getConditionalTime(),
      getConditionalOperatorEnum());
  }

  //! \brief Get the specialized UMAA conditional object that was used to initialize this object
  //! \return The UMAA specialized conditional object
  TimeConditionalType getSpecializedConditional() const {
    return derivedConditional_;
  }

 private:
  TimeConditionalType derivedConditional_;
};

}  // namespace arlcore::umaa::conditional
#endif  // INCLUDE_UMAA_CONDITIONALS_TIMECONDITIONAL_H_
