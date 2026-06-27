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

#ifndef INCLUDE_UMAA_OBJECTIVE_EXECUTORS_CORE_OBJECTIVEEXECUTORCONFIG_H_
#define INCLUDE_UMAA_OBJECTIVE_EXECUTORS_CORE_OBJECTIVEEXECUTORCONFIG_H_

#include <optional>
#include <string>

#include "NumericGuid.h"

namespace arlcore::umaa {

//! \brief Guidance control mode for what downstream service the objective executor should use to control vehicle
//! Movement
//! VECTOR - Use GlobalVectorControl service
//! WAYPOINT - Use WaypointControl service
enum class GuidanceModeEnum {
  VECTOR,
  WAYPOINT
};

//! \brief Runtime configuration settings for objective executors. This struct is an input to the ObjectiveController
//! --- REQUIRED ---
//! domainID - DDS Domain to join
//! qosProfile - DDS Profile to apply
//! qosProfilePath - Relative path to the qosProfile file
//! id - Component identifier to use when interacting with other UMAA services
//! --- OPTIONAL ---
//! vectorProviderId - ID of the downstream resource to use for GlobalVectorControl
//! waypointProviderId - ID of the downstream resource to use for GlobalWaypointControl
//! guidanceMode - Control enum that determines what autopilot service to control the vehicle
struct ObjectiveExecutorConfig {
  int32_t domainId = 0;
  std::string qosProfile = "BuiltinQosLibExp::Generic.StrictReliable";
  std::string qosProfilePath = "";
  arlcore::NumericGuid id = arlcore::NIL_GUID;
  std::string objectiveTopic = "";
  std::optional<arlcore::NumericGuid> vectorProviderId = std::nullopt;
  std::optional<arlcore::NumericGuid> waypointProviderId = std::nullopt;
  std::optional<GuidanceModeEnum> guidanceMode = std::nullopt;
};

}  // namespace arlcore::umaa
#endif  // INCLUDE_UMAA_OBJECTIVE_EXECUTORS_CORE_OBJECTIVEEXECUTORCONFIG_H_
