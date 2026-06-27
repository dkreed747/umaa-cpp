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

#ifndef INCLUDE_UMAA_CONDITIONALS_HEADINGSECTORCONDITIONAL_H_
#define INCLUDE_UMAA_CONDITIONALS_HEADINGSECTORCONDITIONAL_H_

#include <vector>

#include <UMAA/MM/Conditional/HeadingSectorConditionalType.hpp>
#include <UMAA/SA/GlobalPoseStatus/GlobalPoseReportType.hpp>

#include "ConditionalBase.h"
#include "DdsIoTypes.h"
#include "Observer.h"

namespace arlcore::umaa::conditional {

using UMAA::MM::Conditional::HeadingSectorConditionalType;
using UMAA::MM::Conditional::HeadingSectorConditionalTypeTopic;
using UMAA::MM::Conditional::HeadingSectorType;
using UMAA::SA::GlobalPoseStatus::GlobalPoseReportType;
using UMAA::Common::MaritimeEnumeration::HeadingSectorKindEnumModule::HeadingSectorKindEnumType;

using arlcore::Observer;
using arlcore::io::SampleEnvelope;

//! @brief This class defines a heading sector conditional. The conditional is true when all heading sectors in
//! the set are determined to be true; each heading sector is true when the vehicle yaw, provided by
//! GlobalPoseReportType, is either inside or outside the defined sector as indicated by the headingSectorKind.
class HeadingSectorConditional : public ConditionalBase, public Observer<GlobalPoseReportType> {
 public:
  //! \brief Constructor
  //! \param conditional The base UMAA conditional type
  //! \param derivedConditional The specialized HeadingSectorConditional UMAA type
  HeadingSectorConditional(
    const ConditionalType& conditional,
    const HeadingSectorConditionalType& derivedConditional) :
    ConditionalBase(conditional),
    derivedConditional_(derivedConditional) {
    if (!isValidSpecialization(conditional, derivedConditional, HeadingSectorConditionalTypeTopic)) {
      throw std::invalid_argument("Conditional does not match specialization");
    }
  }

  //! \brief Get the heading sectors defined by the conditional
  //! \return A vector containing the heading sectors defined by the conditional
  std::vector<HeadingSectorType> getConditionalSectors() const {
    std::vector<HeadingSectorType> sectors(derivedConditional_.sector().begin(),
      derivedConditional_.sector().end());
    return sectors;
  }

  //! \brief Overridden function to evaluate whether the stored conditional is true or false
  //! \return The evaluation status of the conditional (boolean). If a failure to evaluate the conditional occurs,
  //! the function returns nullopt.
  std::optional<bool> evaluateConditional() const override {
    if (gpData_.has_value()) {
      for (auto it = derivedConditional_.sector().begin(); it != derivedConditional_.sector().end(); ++it) {
        flt64_t yaw = gpData_->attitude().yaw().yaw();
        if (it->headingSectorKind() == HeadingSectorKindEnumType::INSIDE) {
          if (yaw < it->startHeading() || yaw > it->endHeading()) {
            return false;
          }
        } else if (it->headingSectorKind() == HeadingSectorKindEnumType::OUTSIDE) {
          if (yaw > it->startHeading() && yaw < it->endHeading()) {
            return false;
          }
        } else {
          UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Not a valid HeadingSectorKind enum value")
          return std::nullopt;
        }
      }
      return true;
    } else {
      UMAA_LOG_WARN(util::SYSTEM_LOGGER, "No Global Pose data available to evaluate conditional")
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
  HeadingSectorConditionalType getSpecializedConditional() const {
    return derivedConditional_;
  }

 private:
  HeadingSectorConditionalType derivedConditional_;
  std::optional<GlobalPoseReportType> gpData_;
};

}  // namespace arlcore::umaa::conditional
#endif  // INCLUDE_UMAA_CONDITIONALS_HEADINGSECTORCONDITIONAL_H_

