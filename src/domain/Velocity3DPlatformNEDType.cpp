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

#include "Velocity3DPlatformNEDType.h"

namespace arlcore {
  const flt64_t& Velocity3DPlatformNEDType::getDownSpeed() const {
     return downSpeed_;
  }

  const flt64_t& Velocity3DPlatformNEDType::getEastSpeed() const {
    return eastSpeed_;
  }
  const flt64_t& Velocity3DPlatformNEDType::getNorthSpeed() const {
     return northSpeed_;
  }

  void Velocity3DPlatformNEDType::setDownSpeed(const flt64_t& inDownSpeed) {
    downSpeed_ = inDownSpeed;
  }

  void Velocity3DPlatformNEDType::setEastSpeed(const flt64_t& inEastSpeed) {
    eastSpeed_ = inEastSpeed;
  }

  void Velocity3DPlatformNEDType::setNorthSpeed(const flt64_t& inNorthSpeed) {
     northSpeed_ = inNorthSpeed;
  }

}  // namespace arlcore
