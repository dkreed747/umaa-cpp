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

#ifndef INCLUDE_DOMAIN_SPEEDSTATUSDATA_H_
#define INCLUDE_DOMAIN_SPEEDSTATUSDATA_H_

#include "InternalTypes.h"
#include "EnumSpecifiers.h"

namespace arlcore {

class SpeedStatusData {
 public:
  SpeedStatusData() = default;
  virtual ~SpeedStatusData() = default;

  const flt64_t& getSpeedOverGround() const;
  const flt64_t& getSpeedThroughAir() const;
  const flt64_t& getSpeedThroughWater() const;

  flt64_t getSpeedFromTypeSpecifier(const Speeds& specifier) const;

  void setSpeedOverGround(const flt64_t& inSpeed);
  void setSpeedThroughAir(const flt64_t& inSpeed);
  void setSpeedThroughWater(const flt64_t& inSpeed);

 private:
  flt64_t speedOverGround_ = 0.0;
  flt64_t speedThroughAir_ = 0.0;
  flt64_t speedThroughWater_ = 0.0;
};

}  // namespace arlcore

#endif  // INCLUDE_DOMAIN_SPEEDSTATUSDATA_H_
