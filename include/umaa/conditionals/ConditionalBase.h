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

#ifndef INCLUDE_UMAA_CONDITIONALS_CONDITIONALBASE_H_
#define INCLUDE_UMAA_CONDITIONALS_CONDITIONALBASE_H_

#include <string>
#include <memory>
#include <utility>

#include "UMAA/MM/Conditional/ConditionalType.hpp"

#include "Logger.h"
#include "NumericGuid.h"

namespace arlcore::umaa::conditional {

using UMAA::MM::Conditional::ConditionalType;
using UMAA::Common::MaritimeEnumeration::ConditionalOperatorEnumModule::ConditionalOperatorEnumType;

//! \brief Base class for all UMAA conditional specializations to derive from
class ConditionalBase {
 protected:
  //! \brief Explicit constructor takes the UMAA conditional base type
  //! \param conditional UMAA::MM::Conditional::ConditionalType
  explicit ConditionalBase(const ConditionalType& conditional) : baseConditional_(conditional) {}

 public:
  // \brief Default destructor
  virtual ~ConditionalBase() = default;

  //! \brief Get the stored UMAA Conditional type
  //! \return UMAA::MM::Conditional::ConditionalType
  ConditionalType getBaseConditional() const {
    return baseConditional_;
  }

  //! \brief Set the stored UMAA conditional type
  //! \param setConditional The UMAA conditional type to set
  void BaseConditional(const ConditionalType& setConditional) { baseConditional_ = setConditional; }

  //! \brief Get the conditional ID
  //! \return GUID
  arlcore::NumericGuid getConditionalId() const {
    return arlcore::NumericGuid(baseConditional_.conditionalID());
  }

  //! \brief Get the name of the conditional
  //! \return name string
  std::string getName() const {
    return baseConditional_.name();
  }

  //! \brief Get the ID that designates the specialization type
  //! \return GUID
  arlcore::NumericGuid getSpecializationId() const {
    return arlcore::NumericGuid(baseConditional_.specializationID());
  }

  //! \brief Get the specialization timestamp
  //! \return UMAA DateTime
  UMAA::Common::Measurement::DateTime getSpecializationTimestamp() const {
    return baseConditional_.specializationTimestamp();
  }

  //! \brief Get the DDS topic the specialization type is published on
  //! \return UMAA topic string
  std::string getSpecializationTopic() const {
    return baseConditional_.specializationTopic();
  }

  //! \brief Pure virtual function that should be overridden by derivations to evaluate whether the stored conditional
  //! is true or false
  //! \return The evaluation status of the conditional (boolean). If a failure to evaluate the conditional occurs,
  //! the function should return an empty optional.
  virtual std::optional<bool> evaluateConditional() const = 0;

  //! \brief Get the conditional ID(s) of any conditionals that this conditional depends on to be evaluated
  //! \return A pair of optional NumericGUIDs containing the ID(s) of the conditionals that this conditional depends on
  virtual std::pair<std::optional<arlcore::NumericGuid>, std::optional<arlcore::NumericGuid>> getDependencies() const {
    return std::make_pair(std::nullopt, std::nullopt);
  }

  //! \brief Helper function to verify that a specialized conditional matches the generalized type
  //! \tparam Specialization A type that is a specialization of ConditionalType
  //! \param base The ConditionalType to compare
  //! \param derived The specialization type to compare
  //! \param topic The topic of the specialized type that should be present in the ConditionalType
  //! \return Whether or not the pair is a valid specialization
  template <class Specialization>
  static bool isValidSpecialization(const ConditionalType& base, const Specialization& derived,
                                    std::string_view topic) {
    if (base.specializationTopic() != topic) {
      UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Attempted to create conditional with mismatched topic")
      return false;
    }
    return base.specializationID() == derived.specializationReferenceID() &&
      base.specializationTimestamp() == derived.specializationReferenceTimestamp();
  }

  //! \brief Convert a pointer to a ConditionalBase to a pointer of its specialized derivation
  //! \tparam Specialized The specialized type derived from ConditionalBase
  //! \param base The shared pointer to the ConditionalBase
  //! \return A shared pointer to the Specialized representation of 'base'
  template <class Specialized> static std::shared_ptr<Specialized> getSpecialized(
      std::shared_ptr<ConditionalBase> base) {
    return std::static_pointer_cast<Specialized, ConditionalBase>(base);
  }

 protected:
  template <typename T>
  bool compareWithConditionalOp(const T& lhs, const T& rhs, ConditionalOperatorEnumType op) const {
    switch (op) {
      case ConditionalOperatorEnumType::GREATER_THAN:
        return lhs > rhs;
      case ConditionalOperatorEnumType::GREATER_THAN_OR_EQUAL_TO:
        return lhs >= rhs;
      case ConditionalOperatorEnumType::LESS_THAN:
        return lhs < rhs;
      case ConditionalOperatorEnumType::LESS_THAN_OR_EQUAL_TO:
        return lhs <= rhs;
      default:
        UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Attempted to handle Unsupported ConditionalOperatorEnum value: " +
                        static_cast<int32_t>(op))
        throw std::invalid_argument("Attempted to handle Unsupported ConditionalOperatorEnum value: " +
                                    static_cast<int32_t>(op));
    }
  }

 private:
  ConditionalType baseConditional_;
};

}  // namespace arlcore::umaa::conditional
#endif  // INCLUDE_UMAA_CONDITIONALS_CONDITIONALBASE_H_
