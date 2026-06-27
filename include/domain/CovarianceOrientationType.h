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

#ifndef INCLUDE_DOMAIN_COVARIANCEORIENTATIONTYPE_H_
#define INCLUDE_DOMAIN_COVARIANCEORIENTATIONTYPE_H_

#include "InternalTypes.h"

namespace arlcore {

class CovarianceOrientationType {
 public:
  CovarianceOrientationType() = default;
  virtual ~CovarianceOrientationType() = default;

  const flt64_t& getPitchPitchAngleErrCovar() const;
  const flt64_t& getPitchYawAngleErrCovar() const;
  const flt64_t& getRollPitchAngleErrCovar() const;
  const flt64_t& getRollRollAngleErrCovar() const;
  const flt64_t& getRollYawAngleErrCovar() const;
  const flt64_t& getYawYawAngleErrCovar() const;

  void setPitchPitchAngleErrCovar(const flt64_t& inPitchPitchAngleErr);
  void setPitchYawAngleErrCovar(const flt64_t& inPitchYawAngleErr);
  void setRollPitchAngleErrCovar(const flt64_t& inRollPitchAngleErr);
  void setRollRollAngleErrCovar(const flt64_t& inRollRollAngleErr);
  void setRollYawAngleErrCovar(const flt64_t& inRollYawAngleErr);
  void setYawYawAngleErrCovar(const flt64_t& inYawYawAngleErr);

 private:
  flt64_t rpRp_ = 0.0;  // Pitch-Pitch angle-angle error covariance.
                        // Units of radians squared.
  flt64_t rpRy_ = 0.0;  // Pitch-Yaw angle-angle error covariance.
                        // Units of radians squared.
  flt64_t rrRp_ = 0.0;  // Roll-Pitch angle-angle error covariance.
                        // Units of radians squared.
  flt64_t rrRr_ = 0.0;  // Roll-Roll angle-angle error covariance.
                        // Units of radians squared.
  flt64_t rrRy_ = 0.0;  // Roll-Yaw angle-angle error covariance.
                        // Units of radians squared.
  flt64_t ryRy_ = 0.0;  // Yaw-Yaw angle-angle error covariance.
                        // Units of radians squared.
};
}  // namespace arlcore

#endif  // INCLUDE_DOMAIN_COVARIANCEORIENTATIONTYPE_H_
