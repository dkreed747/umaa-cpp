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

#include "OrientationVel3D.h"

namespace arlcore {
  const flt64_t& OrientationVel3D::getPitchRate() const {
     return pitchRate_;
  }

  const flt64_t& OrientationVel3D::getRollRate() const {
    return rollRate_;
  }
  const flt64_t& OrientationVel3D::getYawRate() const {
     return yawRate_;
  }

  void OrientationVel3D::setPitchRate(const flt64_t& inPitchRate) {
    pitchRate_ = inPitchRate;
  }

  void OrientationVel3D::setRollRate(const flt64_t& inRollRate) {
    rollRate_ = inRollRate;
  }

  void OrientationVel3D::setYawRate(const flt64_t& inYawRate) {
     yawRate_ = inYawRate;
  }

}  // namespace arlcore
