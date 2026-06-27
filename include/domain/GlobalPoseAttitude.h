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

#ifndef INCLUDE_DOMAIN_GLOBALPOSEATTITUDE_H_
#define INCLUDE_DOMAIN_GLOBALPOSEATTITUDE_H_

#include "InternalTypes.h"

namespace arlcore {

//! \brief Common Global Position attitude class containing Pitch, Roll, and Yaw
class GlobalPoseAttitude {
 public:
  const flt64_t& getPitch() const;
  const flt64_t& getRoll() const;
  const flt64_t& getYaw() const;

  void setPitch(const flt64_t& inPitch);
  void setRoll(const flt64_t& inRoll);
  void setYaw(const flt64_t& inYaw);

 private:
  flt64_t pitch_ = 0.0;
  flt64_t roll_ = 0.0;
  flt64_t yaw_ = 0.0;
};
}  // namespace arlcore

#endif  // INCLUDE_DOMAIN_GLOBALPOSEATTITUDE_H_
