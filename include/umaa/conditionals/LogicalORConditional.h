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

#ifndef INCLUDE_UMAA_CONDITIONALS_LOGICALORCONDITIONAL_H_
#define INCLUDE_UMAA_CONDITIONALS_LOGICALORCONDITIONAL_H_

#include <memory>
#include <utility>

#include <UMAA/MM/Conditional/LogicalORConditionalType.hpp>

#include "ConditionalBase.h"

namespace arlcore::umaa::conditional {

using UMAA::MM::Conditional::LogicalORConditionalType;
using UMAA::MM::Conditional::LogicalORConditionalTypeTopic;

//! @brief This class defines a logical OR operator for a set of conditionals. The conditional is true when at
//! least one of the conditionals referenced by conditionalID1 and conditionalID2 evaluate to true.
class LogicalORConditional : public ConditionalBase {
 public:
  //! \brief Constructor
  //! \param conditional The base UMAA conditional type
  //! \param derivedConditional The specialized LogicalORConditional UMAA type
  LogicalORConditional(
    const ConditionalType& conditional,
    const LogicalORConditionalType& derivedConditional) :
    ConditionalBase(conditional), derivedConditional_(derivedConditional) {
    if (!isValidSpecialization(conditional, derivedConditional, LogicalORConditionalTypeTopic)) {
      throw std::invalid_argument("Conditional does not match specialization");
    }
  }

  //! \brief Link this conditional to the internal representation its specified conditionals for evaluation
  //! \param first A pointer to the first conditional
  //! \param second A pointer to the second conditional
  void setReferencedConditionals(std::weak_ptr<ConditionalBase> first, std::weak_ptr<ConditionalBase> second) {
    if (auto cond = first.lock()) {
      if (cond->getConditionalId() != derivedConditional_.conditionalID1()) {
        UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Attempted to link conditional with wrong ID")
        return;
      }
    }
    if (auto cond = second.lock()) {
      if (cond->getConditionalId() != derivedConditional_.conditionalID2()) {
        UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Attempted to link conditional with wrong ID")
        return;
      }
    }

    firstConditional_ = first;
    secondConditional_ = second;
  }

  //! \brief Get the conditional IDs specified by the conditional
  //! \return A pair of NumericGuids corresponding to the conditional IDs specified by the conditional
  std::pair<arlcore::NumericGuid, arlcore::NumericGuid> getReferencedConditionalIds() const {
    return {arlcore::NumericGuid(derivedConditional_.conditionalID1()),
      arlcore::NumericGuid(derivedConditional_.conditionalID2())};
  }

  //! \brief Overridden function to evaluate whether the stored conditional is true or false
  //! \return The evaluation status of the conditional (boolean). If a failure to evaluate the conditional occurs,
  //! the function returns nullopt.
  std::optional<bool> evaluateConditional() const override {
    if (auto firstConditional = firstConditional_.lock()) {
      if (auto secondConditional = secondConditional_.lock()) {
        auto c1 = firstConditional->evaluateConditional();
        auto c2 = secondConditional->evaluateConditional();
        if (!c1.has_value() || !c2.has_value()) {
          return std::nullopt;
        }
        return c1.value() || c2.value();
      } else {
        UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Failed to acquire lock on the second conditional weak pointer")
        return std::nullopt;
      }
    } else {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Failed to acquire lock on the first conditional weak pointer")
      return std::nullopt;
    }
  }

  //! \brief Overridden function to get the conditional ID(s) of any conditionals that this conditional depends on to
  //! be evaluated (i.e. the conditionals whose evaluations are ORed together)
  //! \return A pair of optional NumericGUIDs containing the first and second conditional IDs
  std::pair<std::optional<arlcore::NumericGuid>, std::optional<arlcore::NumericGuid>> getDependencies() const override {
    return std::make_pair(arlcore::NumericGuid(derivedConditional_.conditionalID1()),
      arlcore::NumericGuid(derivedConditional_.conditionalID2()));
  }

  //! \brief Get the specialized UMAA conditional object that was used to initialize this object
  //! \return The UMAA specialized conditional object
  LogicalORConditionalType getSpecializedConditional() const {
    return derivedConditional_;
  }

 private:
  LogicalORConditionalType derivedConditional_;
  std::weak_ptr<ConditionalBase> firstConditional_;
  std::weak_ptr<ConditionalBase> secondConditional_;
};

}  // namespace arlcore::umaa::conditional
#endif  // INCLUDE_UMAA_CONDITIONALS_LOGICALORCONDITIONAL_H_
