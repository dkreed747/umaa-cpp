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

#ifndef INCLUDE_HEALTH_STATUS_HEALTHSTATUSDATA_H_
#define INCLUDE_HEALTH_STATUS_HEALTHSTATUSDATA_H_

#include "InternalTypes.h"
#include "Logger.h"
#include "NumericGuid.h"

using UMAA::Common::MaritimeEnumeration::ErrorCodeEnumModule::ErrorCodeEnumType;
using UMAA::Common::MaritimeEnumeration::ErrorConditionEnumModule::ErrorConditionEnumType;

namespace arlcore {


// Corresponding UMAA types output for the Health Report status()
// Local HealthState enum should map to these values,
// although not all may be needed.
/* enum ErrorConditionEnumType {
   ERROR, // An error condition is reported and expected to seriously compromise use of the reporting component or device.
   FAIL, // An error condition is reported with severity indicating component or device failure.
   INFO, // An error condition is reported, but impact on operation and performance is minimal.
   NONE, // No error condition exists.
   WARN // An error condition is reported and expected to have significant impact on component or device performance.
   };
*/
// Re-ordering local enum to facilitate easy comparison for health state.
enum class HealthState {
  HEALTH_NONE,
  HEALTH_INFO,
  HEALTH_WARN,
  HEALTH_ERROR,
  HEALTH_FAIL
};

// Corresponding UMAA types output for Health Report code field.
// Described as "The types of system or subsystems associated with the error report"
// Local HealthSubsystem enum should map to these values,
// although not all may be needed.
/* enum ErrorCodeEnumType {
   ACTUATOR,
   FILESYS,
   NONE,
   POWER,
   PROCESSOR,
   RAM,
   ROM,
   SENSOR,
   SOFTWARE
   };
*/
enum class HealthReportingEntity {
  ACTUATOR,
  FILESYS,
  NONE,
  POWER,
  PROCESSOR,
  RAM,
  ROM,
  SENSOR,
  SOFTWARE
};

//! \brief Common Health Status Data class containing component health information.
class HealthStatusData {
 public:
  //! \brief Construct a new default Health Status object
  HealthStatusData() = default;

  //! \brief Destroy the Health Status object
  virtual ~HealthStatusData() = default;

  ErrorConditionEnumType getSeverity(HealthState state) const {
    switch (state) {
      case HealthState::HEALTH_NONE: return ErrorConditionEnumType::NONE;
      case HealthState::HEALTH_INFO: return ErrorConditionEnumType::INFO;
      case HealthState::HEALTH_WARN: return ErrorConditionEnumType::WARN;
      case HealthState::HEALTH_ERROR: return ErrorConditionEnumType::ERROR;
      case HealthState::HEALTH_FAIL: return ErrorConditionEnumType::FAIL;
      default:
        // ERROR case. Should not be hit unless more fields are added.
        UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Invalid enum for Health Report.")
        return ErrorConditionEnumType::NONE;
    }
  }

  ErrorCodeEnumType getCode(HealthReportingEntity entity) const {
    switch (entity) {
      case HealthReportingEntity::ACTUATOR: return ErrorCodeEnumType::ACTUATOR;
      case HealthReportingEntity::FILESYS: return ErrorCodeEnumType::FILESYS;
      case HealthReportingEntity::NONE: return ErrorCodeEnumType::NONE;
      case HealthReportingEntity::POWER: return ErrorCodeEnumType::POWER;
      case HealthReportingEntity::PROCESSOR: return ErrorCodeEnumType::PROCESSOR;
      case HealthReportingEntity::RAM: return ErrorCodeEnumType::RAM;
      case HealthReportingEntity::ROM: return ErrorCodeEnumType::ROM;
      case HealthReportingEntity::SENSOR: return ErrorCodeEnumType::SENSOR;
      case HealthReportingEntity::SOFTWARE: return ErrorCodeEnumType::SOFTWARE;
      default:
        // ERROR case. Should not be hit unless more fields are added.
        UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Invalid enum for Health Report Code.")
        return ErrorCodeEnumType::NONE;
    }
  }

  static size_t errorCodeToInt(ErrorCodeEnumType code) {
    switch (code) {
      case ErrorCodeEnumType::ACTUATOR: return 0;
      case ErrorCodeEnumType::FILESYS: return 1;
      case ErrorCodeEnumType::NONE: return 2;
      case ErrorCodeEnumType::POWER: return 3;
      case ErrorCodeEnumType::PROCESSOR: return 4;
      case ErrorCodeEnumType::RAM: return 5;
      case ErrorCodeEnumType::ROM: return 6;
      case ErrorCodeEnumType::SENSOR: return 7;
      case ErrorCodeEnumType::SOFTWARE: return 8;
      default: throw std::invalid_argument("Invalid ErrorCodeEnumType");
    }
  }
};
}  // namespace arlcore

#endif  // INCLUDE_HEALTH_STATUS_HEALTHSTATUSDATA_H_
