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

#ifndef INCLUDE_UMAA_CONDITIONALS_CONSTRAINTVIOLATEDCONDITIONAL_H_
#define INCLUDE_UMAA_CONDITIONALS_CONSTRAINTVIOLATEDCONDITIONAL_H_

#include <memory>
#include <optional>
#include <utility>

#include "UMAA/MM/Conditional/ConstraintViolatedConditionalType.hpp"

#include "ConditionalBase.h"
#include "RealtimeSystemClock.h"

namespace arlcore::umaa::conditional {

using UMAA::MM::Conditional::ConstraintViolatedConditionalType;
using UMAA::MM::Conditional::ConstraintViolatedConditionalTypeTopic;

//! \brief This class defines a constraint violated conditional. The conditional is true when the conditional provided
//! in the ConditionalReportType message, as specified by constraintConditionalID, is determined to be false.
class ConstraintViolatedConditional : public ConditionalBase {
 public:
  //! \brief Constructor
  //! \param conditional The base UMAA conditional type
  //! \param derivedConditional The specialized ConstraintViolatedConditional UMAA type
  ConstraintViolatedConditional(
    const ConditionalType& conditional,
    const ConstraintViolatedConditionalType& derivedConditional) :
    ConditionalBase(conditional), derivedConditional_(derivedConditional) {
      if (!isValidSpecialization(conditional, derivedConditional, ConstraintViolatedConditionalTypeTopic)) {
        throw std::invalid_argument("Conditional does not match specialization");
      }
    }

  //! \brief Link this conditional to the internal representation of its constraint conditional for evaluation
  //! \param constraint A pointer to the conditional object to use as a constraint for evaluation
  void setConstraintConditional(std::weak_ptr<ConditionalBase> constraint) {
    if (auto cond = constraint.lock()) {
      if (cond->getConditionalId() != getConstraintConditionalId()) {
        UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Attempted to link constraint conditional with wrong ID")
        return;
      }
    }
    constraintConditional_ = constraint;
  }

  //! \brief Get the conditional ID of the constraint conditional
  //! \return The ID of the constraint conditional
  arlcore::NumericGuid getConstraintConditionalId() const {
    return arlcore::NumericGuid(derivedConditional_.constraintConditionalID());
  }

  //! \brief Get the duration that the constraint conditional needs to be invalid for to be considered violated
  //! \return The duration the constraint needs to be false to be considered violated or nullopt if not provided
  std::optional<flt64_t> getDuration() const {
    return derivedConditional_.duration().has_value() ?
      std::optional<flt64_t>(derivedConditional_.duration().value()) : std::nullopt;
  }

  //! \brief Overridden function to evaluate whether the stored conditional is true or false
  //! \return boolean conditional true or false. If a failure to evaluate the conditional occurs should return nullopt
  std::optional<bool> evaluateConditional() const override {
    static arlcore::RealtimeSystemClock clk;
    static std::optional<arlcore::timestamp> initialFailureTime;
    if (auto constraintConditional = constraintConditional_.lock()) {
      auto eval = constraintConditional->evaluateConditional();
      if (!eval.has_value()) {
        return std::nullopt;
      }
      if (!derivedConditional_.duration().has_value()) {
        // No duration, simple not
        return !eval.value();
      } else {
        if (!eval.value()) {
          auto currentTime = clk.getCurrentTime_timestamp();
          if (initialFailureTime.has_value()) {
            if ((currentTime.seconds + (static_cast<flt64_t>(currentTime.nanoseconds) / arlcore::NANOSECONDS_PER_SEC))
              - (initialFailureTime->seconds + (static_cast<flt64_t>(initialFailureTime->nanoseconds) /
              arlcore::NANOSECONDS_PER_SEC)) >= derivedConditional_.duration().value()) {
              // Conditional was false for duration or longer, violated
              return true;
            }
          } else {
            initialFailureTime = currentTime;
          }
        } else {
          // Reset any failure count if the constraint is achieved
          initialFailureTime.reset();
        }
        // Not violated
        return false;
      }
      return !constraintConditional->evaluateConditional();
    } else {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Failed to acquire lock on the constraint conditional weak pointer")
      return std::nullopt;
    }
  }

  //! \brief Overridden function to get the conditional ID(s) of any conditionals that this conditional depends on to
  //! be evaluated (i.e. the constraint conditional)
  //! \return A pair of optional NumericGUIDs containing the constraint conditional ID and nullopt
  std::pair<std::optional<arlcore::NumericGuid>, std::optional<arlcore::NumericGuid>> getDependencies() const override {
    return std::make_pair(arlcore::NumericGuid(derivedConditional_.constraintConditionalID()), std::nullopt);
  }

  //! \brief Get the specialized UMAA conditional object that was used to initialize this object
  //! \return The UMAA specialized conditional object
  ConstraintViolatedConditionalType getSpecializedConditional() const {
    return derivedConditional_;
  }

 private:
  ConstraintViolatedConditionalType derivedConditional_;
  std::weak_ptr<ConditionalBase> constraintConditional_;
};

}  // namespace arlcore::umaa::conditional
#endif  // INCLUDE_UMAA_CONDITIONALS_CONSTRAINTVIOLATEDCONDITIONAL_H_
