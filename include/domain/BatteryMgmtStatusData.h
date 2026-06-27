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

#ifndef INCLUDE_DOMAIN_BATTERYMGMTSTATUSDATA_H_
#define INCLUDE_DOMAIN_BATTERYMGMTSTATUSDATA_H_

#include <vector>

#include "InternalTypes.h"
#include "BatteryStatusData.h"

namespace arlcore {

class BatteryMgmtStatusData {
 public:
  BatteryMgmtStatusData() = default;
  virtual ~BatteryMgmtStatusData() = default;

  std::vector<BatteryStatusData> getBatteries() const {
    return batteries_;
  }

  int32_t getNumberOfBatteries() const {
    return batteries_.size();
  }

  void addBattery(const BatteryStatusData& battery) {
    batteries_.push_back(battery);
  }

 private:
  std::vector<BatteryStatusData> batteries_;
};

}  // namespace arlcore
#endif  // INCLUDE_DOMAIN_BATTERYMGMTSTATUSDATA_H_
