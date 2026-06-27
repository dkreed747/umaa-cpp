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

#ifndef INCLUDE_DOMAIN_BATTERYSTATUSDATA_H_
#define INCLUDE_DOMAIN_BATTERYSTATUSDATA_H_

#include "InternalTypes.h"
#include "EnumSpecifiers.h"

namespace arlcore {

//! \brief Common Battery Status data class
class BatteryStatusData {
 public:
  BatteryStatusData() = default;
  virtual ~BatteryStatusData() = default;

  void setVoltage(const flt64_t& inVol) {
    voltage = inVol;
  }

  void setTemp(const flt64_t& inTemp) {
    temp = inTemp;
  }

  void setCurrent(const flt64_t& inCurrent) {
    current = inCurrent;
  }

  void setCharge(const flt64_t& inCharge) {
    charge = inCharge;
  }

  const flt64_t& getVoltage() const {
    return voltage;
  }

  const flt64_t& getTemp() const {
    return temp;
  }

  const flt64_t& getCurrent() const {
    return current;
  }

  const flt64_t& getCharge() const {
    return charge;
  }

 private:
  flt64_t voltage = 0.0;
  flt64_t temp = 0.0;
  flt64_t current = 0.0;

  flt64_t charge = 0.0;
};

}  // namespace arlcore
#endif  // INCLUDE_DOMAIN_BATTERYSTATUSDATA_H_
