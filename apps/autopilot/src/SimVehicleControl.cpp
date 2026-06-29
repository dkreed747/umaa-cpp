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

#include "SimVehicleControl.h"

#include <optional>

#include "Logger.h"

namespace arlcore::autopilot {

using UMAA::EO::UVPlatformSpecs::UVPlatformSpecsReportType;
using UMAA::EO::UVPlatformSpecs::UVPlatformCapabilitiesReportType;
using UMAA::EO::UVPlatformSpecs::SurfaceCapabilityLimitsType;
using UMAA::EO::UVPlatformSpecs::UnderwaterCapabilityLimitsType;

namespace {
//! \brief Copy an optional config value into a generated @optional field.
void setIf(std::optional<double>& field, const std::optional<double>& value) {  // NOLINT(runtime/references)
  if (value.has_value()) {
    field = value.value();
  }
}
}  // namespace

SimVehicleControl::SimVehicleControl(const PlatformSpecsConfig& specs,
                                     const PlatformCapabilitiesConfig& caps) :
    specs_(specs), caps_(caps) {}

bool SimVehicleControl::initialize() {
  UMAA_LOG_INFO(util::SYSTEM_LOGGER, "SimVehicleControl initialized for platform: " << specs_.name)
  return true;
}

bool SimVehicleControl::sendControlVector(const ControlVector& cv) {
  lastControlVector_ = cv;
  controlVectorCount_++;
  UMAA_LOG_DEBUG(util::SYSTEM_LOGGER, "SimVehicleControl control vector: heading(rad)=" << cv.headingRad
    << " speed(mps)=" << cv.speedMps
    << " elevation=" << (cv.elevationM.has_value() ? cv.elevationM.value() : 0.0))
  return true;
}

UVPlatformSpecsReportType SimVehicleControl::getPlatformSpecs() const {
  UVPlatformSpecsReportType report;
  report.name() = specs_.name;
  report.lengthAtWaterline() = specs_.lengthAtWaterlineM;
  report.beamAtWaterline() = specs_.beamAtWaterlineM;
  report.draft() = specs_.draftM;
  report.forwardDistance() = specs_.forwardDistanceM;
  report.aftDistance() = specs_.aftDistanceM;
  report.portDistance() = specs_.portDistanceM;
  report.starboardDistance() = specs_.starboardDistanceM;
  report.topDistance() = specs_.topDistanceM;
  report.bottomDistance() = specs_.bottomDistanceM;
  report.displacement() = specs_.displacementMetricTon;
  report.weightLight() = specs_.weightLightMetricTon;
  report.weightLoaded() = specs_.weightLoadedMetricTon;
  // centerOfBuoyancy, centerOfGravity, and referenceFrameOrigin keep generated defaults.
  return report;
}

UVPlatformCapabilitiesReportType SimVehicleControl::getPlatformCapabilities() const {
  UVPlatformCapabilitiesReportType report;
  report.minWaterDepth() = caps_.minWaterDepthM;

  SurfaceCapabilityLimitsType surface;
  setIf(surface.maxForwardSpeed(), caps_.surface.maxForwardSpeedMps);
  setIf(surface.maxReverseSpeed(), caps_.surface.maxReverseSpeedMps);
  setIf(surface.cruisingSpeed(), caps_.surface.cruisingSpeedMps);
  setIf(surface.maxTurnRate(), caps_.surface.maxTurnRateRps);
  setIf(surface.minSpeedInMedium(), caps_.surface.minSpeedInMediumMps);
  report.surfaceCapabilities() = surface;

  if (caps_.underwaterEnabled) {
    UnderwaterCapabilityLimitsType underwater;
    setIf(underwater.maxForwardSpeed(), caps_.underwater.maxForwardSpeedMps);
    setIf(underwater.maxReverseSpeed(), caps_.underwater.maxReverseSpeedMps);
    setIf(underwater.cruisingSpeed(), caps_.underwater.cruisingSpeedMps);
    setIf(underwater.maxTurnRate(), caps_.underwater.maxTurnRateRps);
    setIf(underwater.minSpeedInMedium(), caps_.underwater.minSpeedInMediumMps);
    setIf(underwater.maxDepthChangeRate(), caps_.underwater.maxDepthChangeRateMps);
    report.underwaterCapabilities() = underwater;
  }

  return report;
}

}  // namespace arlcore::autopilot
