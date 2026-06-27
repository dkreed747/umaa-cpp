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

#ifndef INCLUDE_DOMAIN_VELOCITY3DPLATFORMNEDTYPE_H_
#define INCLUDE_DOMAIN_VELOCITY3DPLATFORMNEDTYPE_H_

#include "InternalTypes.h"

namespace arlcore {

class Velocity3DPlatformNEDType {
 public:
  Velocity3DPlatformNEDType() = default;
  virtual ~Velocity3DPlatformNEDType() = default;

  const flt64_t& getDownSpeed() const;
  const flt64_t& getEastSpeed() const;
  const flt64_t& getNorthSpeed() const;

  void setDownSpeed(const flt64_t& inDownSpeed);
  void setEastSpeed(const flt64_t& inEastSpeed);
  void setNorthSpeed(const flt64_t& inNorthSpeed);

 private:
  flt64_t downSpeed_ = 0.0;
  flt64_t eastSpeed_ = 0.0;
  flt64_t northSpeed_ = 0.0;
};
}  // namespace arlcore

#endif  // INCLUDE_DOMAIN_VELOCITY3DPLATFORMNEDTYPE_H_
