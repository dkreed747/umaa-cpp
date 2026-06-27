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

#ifndef INCLUDE_DOMAIN_COVARIANCEVELOCITYTYPE_H_
#define INCLUDE_DOMAIN_COVARIANCEVELOCITYTYPE_H_

#include "InternalTypes.h"

namespace arlcore {

class CovarianceVelocityType {
 public:
  CovarianceVelocityType() = default;
  virtual ~CovarianceVelocityType() = default;

  const flt64_t& getDownDownVelErrCovar() const;
  const flt64_t& getEastDownVelErrCovar() const;
  const flt64_t& getEastEastVelErrCovar() const;
  const flt64_t& getNorthDownVelErrCovar() const;
  const flt64_t& getNorthEastVelErrCovar() const;
  const flt64_t& getNorthNorthVelErrCovar() const;

  void setDownDownVelErrCovar(const flt64_t& inDownDownVelErr);
  void setEastDownVelErrCovar(const flt64_t& inEastDownVelErr);
  void setEastEastVelErrCovar(const flt64_t& inEastEastVelErr);
  void setNorthDownVelErrCovar(const flt64_t& inNorthDownVelErr);
  void setNorthEastVelErrCovar(const flt64_t& inNorthEastVelErr);
  void setNorthNorthVelErrCovar(const flt64_t& inNorthNorthVelErr);

 private:
  flt64_t vdVd_ = 0.0;  // Down-Down velocity-velocity error covariance.
                        // Meters squared per second squared.
  flt64_t veVd_ = 0.0;  // East-Down velocity-velocity error covariance.
                        // Meters squared per second squared.
  flt64_t veVe_ = 0.0;  // East-East velocity-velocity error covariance.
                        // Meters squared per second squared.
  flt64_t vnVd_ = 0.0;  // North-Down velocity-velocity error covariance.
                        // Meters squared per second squared.
  flt64_t vnVe_ = 0.0;  // North-East velocity-velocity error covariance.
                        // Meters squared per second squared.
  flt64_t vnVn_ = 0.0;  // North-North velocity-velocity error covariance.
                        // Meters squared per second squared.
};
}  // namespace arlcore

#endif  // INCLUDE_DOMAIN_COVARIANCEVELOCITYTYPE_H_
