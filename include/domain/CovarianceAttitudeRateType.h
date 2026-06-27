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

#ifndef INCLUDE_DOMAIN_COVARIANCEATTITUDERATETYPE_H_
#define INCLUDE_DOMAIN_COVARIANCEATTITUDERATETYPE_H_

#include "InternalTypes.h"

namespace arlcore {

class CovarAttitudeRateType {
 public:
  CovarAttitudeRateType() = default;
  virtual ~CovarAttitudeRateType() = default;

  const flt64_t& getPitchPitchAttRateErrCovar() const;
  const flt64_t& getPitchYawAttRateErrCovar() const;
  const flt64_t& getRollPitchAttRateErrCovar() const;
  const flt64_t& getRollRollAttRateErrCovar() const;
  const flt64_t& getRollYawAttRateErrCovar() const;
  const flt64_t& getYawYawAttRateErrCovar() const;

  void setPitchPitchAttRateErrCovar(const flt64_t& inPitchPitchRateErr);
  void setPitchYawAttRateErrCovar(const flt64_t& inPitchYawRateErr);
  void setRollPitchAttRateErrCovar(const flt64_t& inRollPitchRateErr);
  void setRollRollAttRateErrCovar(const flt64_t& inRollRollRateErr);
  void setRollYawAttRateErrCovar(const flt64_t& inRollYawRateErr);
  void setYawYawAttRateErrCovar(const flt64_t& inYawYawRateErr);

 private:
  flt64_t rpRp_ = 0.0;  // Pitch-Pitch attitude rate error covariance.
                        // Radians squared per second squared.
  flt64_t rpRy_ = 0.0;  // Pitch-Yaw attitude rate error covariance.
                        // Radians squared per second squared.
  flt64_t rrRp_ = 0.0;  // Roll-Pitch attitude rate error covariance.
                        // Radians squared per second squared.
  flt64_t rrRr_ = 0.0;  // Roll-Roll attitude rate error covariance.
                        // Radians squared per second squared.
  flt64_t rrRy_ = 0.0;  // Roll-Yaw attitude rate error covariance.
                        // Radians squared per second squared.
  flt64_t ryRy_ = 0.0;  // Yaw-Yaw attitude rate error covariance.
                        // Radians squared per second squared.
};
}  // namespace arlcore

#endif  // INCLUDE_DOMAIN_COVARIANCEATTITUDERATETYPE_H_
