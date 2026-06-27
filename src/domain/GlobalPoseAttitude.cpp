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

#include "GlobalPoseAttitude.h"

namespace arlcore {
  const flt64_t& GlobalPoseAttitude::getRoll() const {
     return roll_;
  }

  const flt64_t& GlobalPoseAttitude::getPitch() const {
    return pitch_;
  }
  const flt64_t& GlobalPoseAttitude::getYaw() const {
     return yaw_;
  }

  void GlobalPoseAttitude::setPitch(const flt64_t& inPitch) {
    pitch_ = inPitch;
  }

  void GlobalPoseAttitude::setRoll(const flt64_t& inRoll) {
    roll_ = inRoll;
  }

  void GlobalPoseAttitude::setYaw(const flt64_t& inYaw) {
     yaw_ = inYaw;
  }

}  // namespace arlcore
