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

#ifndef INCLUDE_DOMAIN_ORIENTATIONVEL3D_H_
#define INCLUDE_DOMAIN_ORIENTATIONVEL3D_H_

#include "InternalTypes.h"

namespace arlcore {

class OrientationVel3D {
 public:
  OrientationVel3D() = default;
  virtual ~OrientationVel3D() = default;

  const flt64_t& getPitchRate() const;
  const flt64_t& getRollRate() const;
  const flt64_t& getYawRate() const;

  void setPitchRate(const flt64_t& inPitchRate);
  void setRollRate(const flt64_t& inRollRate);
  void setYawRate(const flt64_t& inYawRate);

 private:
  flt64_t pitchRate_ = 0.0;
  flt64_t rollRate_ = 0.0;
  flt64_t yawRate_ = 0.0;
};
}  // namespace arlcore

#endif  // INCLUDE_DOMAIN_ORIENTATIONVEL3D_H_
