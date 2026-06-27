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

#ifndef INCLUDE_UMAA_CONDITIONALS_LOGICALNOTCONDITIONAL_H_
#define INCLUDE_UMAA_CONDITIONALS_LOGICALNOTCONDITIONAL_H_

#include <memory>
#include <utility>

#include <UMAA/MM/Conditional/LogicalNOTConditionalType.hpp>

#include "ConditionalBase.h"

namespace arlcore::umaa::conditional {

using UMAA::MM::Conditional::LogicalNOTConditionalType;
using UMAA::MM::Conditional::LogicalNOTConditionalTypeTopic;

//! @brief This class defines a logical NOT operator for a conditional. The conditional is true when the conditional
//! referenced by notConditionalID evaluates to false.
class LogicalNOTConditional : public ConditionalBase {
 public:
  //! \brief Constructor
  //! \param conditional The base UMAA conditional type
  //! \param derivedConditional The specialized LogicalNOTConditional UMAA type
  LogicalNOTConditional(
    const ConditionalType& conditional,
    const LogicalNOTConditionalType& derivedConditional) :
    ConditionalBase(conditional), derivedConditional_(derivedConditional) {
    if (!isValidSpecialization(conditional, derivedConditional, LogicalNOTConditionalTypeTopic)) {
      throw std::invalid_argument("Conditional does not match specialization");
    }
  }

  //! \brief Link this conditional to the internal representation of the 'not' conditional specified
  //! \param negated A pointer to the conditional object whose evaluation should be negated
  void setNegatedConditional(std::weak_ptr<ConditionalBase> negated) {
    if (auto cond = negated.lock()) {
      if (cond->getConditionalId() != getNotConditionalId()) {
        UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Attempted to link \"not\" conditional with wrong ID")
        return;
      }
    }
    negatedConditional_ = negated;
  }

  //! \brief Get the conditional ID of the 'not' conditional
  //! \return The ID of the 'not' conditional
  arlcore::NumericGuid getNotConditionalId() const {
    return arlcore::NumericGuid(derivedConditional_.notConditionalID());
  }

  //! \brief Overridden function to evaluate whether the stored conditional is true or false
  //! \return The evaluation status of the conditional (boolean). If a failure to evaluate the conditional occurs,
  //! the function returns nullopt.
  std::optional<bool> evaluateConditional() const override {
    if (auto negatedConditional = negatedConditional_.lock()) {
      auto eval = negatedConditional->evaluateConditional();
      if (!eval.has_value()) {
        return std::nullopt;
      }
      return !eval.value();
    } else {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Failed to acquire lock on the not conditional weak pointer")
      return std::nullopt;
    }
  }

  //! \brief Overridden function to get the conditional ID(s) of any conditionals that this conditional depends on to
  //! be evaluated (i.e. the 'not' conditional)
  //! \return A pair of optional NumericGUIDs containing the 'not' conditional ID and nullopt
  std::pair<std::optional<arlcore::NumericGuid>, std::optional<arlcore::NumericGuid>> getDependencies() const override {
    return std::make_pair(arlcore::NumericGuid(derivedConditional_.notConditionalID()), std::nullopt);
  }

  //! \brief Get the specialized UMAA conditional object that was used to initialize this object
  //! \return The UMAA specialized conditional object
  LogicalNOTConditionalType getSpecializedConditional() const {
    return derivedConditional_;
  }

 private:
  LogicalNOTConditionalType derivedConditional_;
  std::weak_ptr<ConditionalBase> negatedConditional_;
};

}  // namespace arlcore::umaa::conditional
#endif  // INCLUDE_UMAA_CONDITIONALS_LOGICALNOTCONDITIONAL_H_

