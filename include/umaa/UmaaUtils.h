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

#ifndef INCLUDE_UMAA_UMAAUTILS_H_
#define INCLUDE_UMAA_UMAAUTILS_H_

#include <functional>
#include <utility>
#include <tuple>
#include <memory>
#include <UMAA/Common/Measurement/Measurements.hpp>
#include <UMAA/MM/BaseType/ObjectiveType.hpp>
#include <UMAA/MM/Conditional/ConditionalType.hpp>
#include <UMAA/MM/Constraint/ConstraintType.hpp>

#include "NumericGuid.h"

using UMAA::Common::Measurement::DateTime;
using UMAA::MM::BaseType::ObjectiveType;
using UMAA::MM::Conditional::ConditionalType;
using UMAA::Common::Measurement::NumericGUID;
using UMAA::MM::Constraint::ConstraintType;

namespace UMAA::Common::Measurement {

//! \brief Comparison operator overload for > (greater than)
//! \param lhs The left hand side of the comparison
//! \param rhs The right hand side of the comparison
//! \return true if the left hand side is greater than the right hand side, false otherwise
bool operator>(const DateTime &lhs, const DateTime &rhs);

//! \brief Comparison operator overload for >= (greater than or equal to)
//! \param lhs The left hand side of the comparison
//! \param rhs The right hand side of the comparison
//! \return true if the left hand side is greater than or equal to the right hand side, false otherwise
bool operator>=(const DateTime &lhs, const DateTime &rhs);

//! \brief Comparison operator overload for < (less than)
//! \param lhs The left hand side of the comparison
//! \param rhs The right hand side of the comparison
//! \return true if the left hand side is less than the right hand side, false otherwise
bool operator<(const DateTime &lhs, const DateTime &rhs);

//! \brief Comparison operator overload for <= (less than or equal to)
//! \param lhs The left hand side of the comparison
//! \param rhs The right hand side of the comparison
//! \return true if the left hand side is less than or equal to the right hand side, false otherwise
bool operator<=(const DateTime &lhs, const DateTime &rhs);

}  // namespace UMAA::Common::Measurement

namespace UMAA::MM::Conditional {

//! \brief Comparison operator overload for < (less than)
//! \param lhs The left hand side of the comparison
//! \param rhs The right hand side of the comparison
//! \return true if the left hand side is less than the right hand side, false otherwise
bool operator<(const ConditionalType &lhs, const ConditionalType &rhs);

}  // namespace UMAA::MM::Conditional

namespace arlcore::umaa {

//! \brief Return a UMAA DateTime object populated with the current system time
//! \return A DateTime object with the current time
DateTime getTimestamp();

//! \brief A simple function to hash NumericGUIDs into size_t
//! @param guid The NumericGUID to hash
//! @return A size_t that is a hash of the NumericGUID
size_t hashNumericGuid(const NumericGUID& guid);

//! \brief A hash struct for use with Large Set SetElements. Does not require specialization.
//! \tparam SetElement The type of the topic in which Elements are written containing an ID and other metadata
template <class SetElement>
struct SetElementHasher {
  std::size_t operator()(const SetElement& e) const {
    return hashNumericGuid(e.elementID());
  }
};

//! \brief A hash struct for use with Large Set elements. The default behavior returns a constant value and relies
//!        on chaining by default. Ideally this would be specialized for each element type to improve performance
//!        in large Large Sets.
//! \tparam Element The type of elements that the Large Set contains
template <class Element>
struct ElementHasher {
  std::size_t operator()(const Element& e) const {
    return sizeof(e);
  }
};

template <>
struct ElementHasher<ObjectiveType> {
  std::size_t operator()(const ObjectiveType& e) const {
    return hashNumericGuid(e.objectiveID());
  }
};

template <>
struct ElementHasher<ConditionalType> {
  std::size_t operator()(const ConditionalType& e) const {
    return hashNumericGuid(e.conditionalID());
  }
};

}  // namespace arlcore::umaa

template <>
struct std::hash<UMAA::Common::Measurement::NumericGUID> {
  size_t operator()(const UMAA::Common::Measurement::NumericGUID& id) const noexcept {
    size_t upper_bytes;
    size_t lower_bytes;

    std::memcpy(&upper_bytes, id.data(), sizeof(size_t));
    std::memcpy(&lower_bytes, id.data() + sizeof(size_t), sizeof(size_t));

    return upper_bytes + lower_bytes;
  }
};

template<>
struct std::hash<std::pair<NumericGUID, NumericGUID>> {
  size_t operator()(const pair<NumericGUID, NumericGUID>& key) const {
    size_t h1 = arlcore::umaa::hashNumericGuid(key.first);
    size_t h2 = arlcore::umaa::hashNumericGuid(key.second);
    return h1 ^ (h2 << 3);
  }
};

template<>
struct std::hash<std::tuple<NumericGUID, NumericGUID, NumericGUID>> {
  size_t operator()(const std::tuple<NumericGUID, NumericGUID, NumericGUID>& key) const {
    size_t h1 = arlcore::umaa::hashNumericGuid(std::get<0>(key));
    size_t h2 = arlcore::umaa::hashNumericGuid(std::get<1>(key));
    size_t h3 = arlcore::umaa::hashNumericGuid(std::get<2>(key));
    return h1 ^ h2 ^ (h3 << 1);
  }
};

template <>
struct std::hash<std::shared_ptr<ConditionalType>> {
  size_t operator()(const shared_ptr<ConditionalType>& conditional) const noexcept {
    return std::hash<UMAA::Common::Measurement::NumericGUID>{}(conditional->conditionalID());
  }
};

template <>
struct std::hash<std::shared_ptr<ConstraintType>> {
  size_t operator()(const shared_ptr<ConstraintType>& constraint) const noexcept {
    return std::hash<UMAA::Common::Measurement::NumericGUID>{}(constraint->constraintID());
  }
};


#endif  // INCLUDE_UMAA_UMAAUTILS_H_
