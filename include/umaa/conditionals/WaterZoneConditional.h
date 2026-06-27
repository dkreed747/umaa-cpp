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

#ifndef INCLUDE_UMAA_CONDITIONALS_WATERZONECONDITIONAL_H_
#define INCLUDE_UMAA_CONDITIONALS_WATERZONECONDITIONAL_H_

#include <algorithm>
#include <utility>
#include <vector>

#include <UMAA/MM/Conditional/WaterZoneConditionalType.hpp>
#include <UMAA/SA/GlobalPoseStatus/GlobalPoseReportType.hpp>

#include "ConditionalBase.h"
#include "DdsIoTypes.h"
#include "GeographicUtils.h"
#include "Observer.h"

namespace arlcore::umaa::conditional {

using UMAA::MM::Conditional::WaterZoneConditionalType;
using UMAA::MM::Conditional::WaterZoneConditionalTypeTopic;
using UMAA::Common::MaritimeEnumeration::WaterZoneKindEnumModule::WaterZoneKindEnumType;
using UMAA::Common::Measurement::ElevationVariantType;
using UMAA::Common::Measurement::ElevationVariantTypeEnum;
using UMAA::Common::Measurement::ElevationVariantTypeUnion;
using UMAA::Common::Measurement::GeoPosition2D;
using UMAA::Common::Enumeration::LineSegmentEnumModule::LineSegmentEnumType;
using UMAA::MM::BaseType::ShapeVariantType;
using UMAA::SA::GlobalPoseStatus::GlobalPoseReportType;

using arlcore::Observer;
using arlcore::io::SampleEnvelope;

const flt64_t EARTH_RADIUS_METERS = 6371000;

//! \brief This class defines a waterZone conditional. The conditional is true when the current waterZone, provided
//! in GlobalPoseReportType, has the relationship specified in conditionalOp to the specified waterZone.
class WaterZoneConditional : public ConditionalBase, public Observer<GlobalPoseReportType> {
 public:
  //! \brief Constructor
  //! \param conditional The base UMAA conditional type
  //! \param derivedConditional The specialized WaterZoneConditional UMAA type
  WaterZoneConditional(
    const ConditionalType& conditional,
    const WaterZoneConditionalType& derivedConditional) :
    ConditionalBase(conditional),
    derivedConditional_(derivedConditional) {
    if (!isValidSpecialization(conditional, derivedConditional, WaterZoneConditionalTypeTopic)) {
      throw std::invalid_argument("Conditional does not match specialization");
    }
  }

  //! \brief Get the water zone kind specified by the conditional (i.e. INSIDE/OUTSIDE zones)
  //! \return The water zone kind
  WaterZoneKindEnumType getConditionalWaterZoneKind() const {
    return derivedConditional_.zoneKind();
  }

  //! \brief Get the ceiling elevation used for all zones
  //! \return Ceiling elevation in a union of possible units with discriminator _d()
  ElevationVariantTypeUnion getCeiling() const {
    return derivedConditional_.ceiling().ElevationVariantTypeSubtypes();
  }

  //! \brief Get the floor elevation used for all zones
  //! \return Floor elevation in a union of possible units with discriminator _d()
  ElevationVariantTypeUnion getFloor() const {
    return derivedConditional_.floor().ElevationVariantTypeSubtypes();
  }

  //! \brief Get all water zones specified in the conditional
  //! \return A vector of ShapeVariantTypes consisting of all zones that are part of the conditional
  std::vector<ShapeVariantType> getZones() const {
    std::vector<ShapeVariantType> zones;
    std::copy(derivedConditional_.zone().begin(), derivedConditional_.zone().end(), std::back_inserter(zones));
    return std::move(zones);
  }

  //! \brief Overridden function to evaluate whether the stored conditional is true or false
  //! \return The evaluation status of the conditional (boolean). If a failure to evaluate the conditional occurs,
  //! the function returns nullopt.
  std::optional<bool> evaluateConditional() const override {
    if (!gpData_.has_value()) {
      UMAA_LOG_INFO(util::SYSTEM_LOGGER, "No global pose data to evaluate conditional")
      return std::nullopt;
    }
    auto zoneKind = derivedConditional_.zoneKind();

    auto elevationCheck = isWithinElevationRange(derivedConditional_.ceiling(), derivedConditional_.floor(),
      gpData_.value());
    if (!elevationCheck.has_value()) {
      return std::nullopt;
    }
    // If we're outside the elevation requirements, we won't be inside any zones
    if (!elevationCheck.value()) {
      return zoneKind == WaterZoneKindEnumType::OUTSIDE;
    }

    bool allInside = true;
    bool allOutside = true;
    for (auto it = derivedConditional_.zone().begin(); it != derivedConditional_.zone().end(); ++it) {
      if (it->ShapeVariantTypeSubtypes()._d() == UMAA::MM::BaseType::ShapeVariantTypeEnum::ELLIPSEVARIANT_D) {
        if (isInsideEllipse(it->ShapeVariantTypeSubtypes().EllipseVariantVariant(), gpData_->position())) {
          allOutside = false;
        } else {
          allInside = false;
        }
      } else if (it->ShapeVariantTypeSubtypes()._d() == UMAA::MM::BaseType::ShapeVariantTypeEnum::POLYGONVARIANT_D) {
        if (isInsidePolygon(it->ShapeVariantTypeSubtypes().PolygonVariantVariant(), gpData_->position())) {
          allOutside = false;
        } else {
          allInside = false;
        }
      } else {
        UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Invalid/Unsupported Shape Type WaterZone")
        return std::nullopt;
      }
    }

    // If we're inside or outside a zone we're not supposed to be
    if (!allOutside && zoneKind == WaterZoneKindEnumType::OUTSIDE ||
        !allInside && zoneKind == WaterZoneKindEnumType::INSIDE) {
      return false;
    }

    return true;
  }

  //! \brief Overridden function that allows the conditional to act as an observer for global pose data
  //! \param gpData Updated global pose data
  void update(const GlobalPoseReportType &gpData) override {
    gpData_ = gpData;
  }

  //! \brief Get the specialized UMAA conditional object that was used to initialize this object
  //! \return The UMAA specialized conditional object
  WaterZoneConditionalType getSpecializedConditional() const {
    return derivedConditional_;
  }

 private:
  WaterZoneConditionalType derivedConditional_;
  std::optional<GlobalPoseReportType> gpData_;
};

}  // namespace arlcore::umaa::conditional
#endif  // INCLUDE_UMAA_CONDITIONALS_WATERZONECONDITIONAL_H_
