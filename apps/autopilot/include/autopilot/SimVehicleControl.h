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

#ifndef APPS_AUTOPILOT_INCLUDE_AUTOPILOT_SIMVEHICLECONTROL_H_
#define APPS_AUTOPILOT_INCLUDE_AUTOPILOT_SIMVEHICLECONTROL_H_

#include <cstdint>
#include <optional>

#include "AutopilotConfig.h"
#include "IVehicleControl.h"

namespace arlcore::autopilot {

//! \brief A loopback vehicle-control strategy used for bring-up and testing. It records the
//! last control vector it was given and serves the platform specs/capabilities loaded from
//! configuration. Swap this for a real hardware strategy via vehicle_control.type in the YAML.
class SimVehicleControl : public IVehicleControl {
 public:
  SimVehicleControl(const PlatformSpecsConfig& specs, const PlatformCapabilitiesConfig& caps);

  bool initialize() override;
  bool sendControlVector(const ControlVector& cv) override;
  UMAA::EO::UVPlatformSpecs::UVPlatformSpecsReportType getPlatformSpecs() const override;
  UMAA::EO::UVPlatformSpecs::UVPlatformCapabilitiesReportType getPlatformCapabilities() const override;

  //! \brief The most recent control vector handed to the strategy (for tests/diagnostics).
  std::optional<ControlVector> lastControlVector() const { return lastControlVector_; }

  //! \brief Count of control vectors received (for tests/diagnostics).
  uint64_t controlVectorCount() const { return controlVectorCount_; }

 private:
  PlatformSpecsConfig specs_;
  PlatformCapabilitiesConfig caps_;
  std::optional<ControlVector> lastControlVector_;
  uint64_t controlVectorCount_ = 0;
};

}  // namespace arlcore::autopilot
#endif  // APPS_AUTOPILOT_INCLUDE_AUTOPILOT_SIMVEHICLECONTROL_H_
