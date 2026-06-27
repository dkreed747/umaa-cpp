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

#ifndef INCLUDE_UMAA_SPECIALIZATIONS_DATA_SENDABLESPECIALIZATIONBASE_H_
#define INCLUDE_UMAA_SPECIALIZATIONS_DATA_SENDABLESPECIALIZATIONBASE_H_

#include <memory>
#include <string>

#include <UMAA/Common/Measurement/Measurements.hpp>

#include "SenderBase.h"

using arlcore::io::SendStatus;
using UMAA::Common::Measurement::DateTime;
using UMAA::Common::Measurement::NumericGUID;

namespace arlcore::umaa {

//! Base class for immutable Sendable Specialization objects.
class SendableSpecializationBase {
 public:
  virtual ~SendableSpecializationBase() = default;
  //! \brief Since Sendable Specializations are designed to send out their own contents, copies of implementations
  //! should not affect each other. Implement this function to return a shared pointer to a Sendable Specialization with
  //! the given SpecializationID and SpecializationTimestamp.
  virtual std::shared_ptr<SendableSpecializationBase> with(const NumericGUID& specID,
                                                           const DateTime& timestamp) const = 0;
  //! \brief Implement to send the contents of the sendable specialization.
  virtual SendStatus send() = 0;
  //! \brief Implement to dispose the contents of the sendable specialization.
  virtual SendStatus dispose() = 0;
  //! \brief Implement to get the specializationID of the stored specialization.
  virtual UMAA::Common::Measurement::NumericGUID getSpecializationID() = 0;
  //! \brief Implement to get the specializationTimestamp of the stored specialization.
  virtual DateTime getSpecializationTimestamp() = 0;
  //! \brief Implement to get the specialization topic of the stored specialization.
  virtual std::string getSpecializationTopic() = 0;
};
}  // namespace arlcore::umaa
#endif  // INCLUDE_UMAA_SPECIALIZATIONS_DATA_SENDABLESPECIALIZATIONBASE_H_
