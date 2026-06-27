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

#ifndef INCLUDE_DOMAIN_VELOCITYSTATUSDATA_H_
#define INCLUDE_DOMAIN_VELOCITYSTATUSDATA_H_

#include "InternalTypes.h"
#include "OrientationVel3D.h"
#include "CovarianceAttitudeRateType.h"
#include "Velocity3DPlatformNEDType.h"
#include "CovarianceVelocityType.h"

namespace arlcore {

class VelocityStatusData {
 public:
  VelocityStatusData() = default;
  virtual ~VelocityStatusData() = default;

  const arlcore::OrientationVel3D& getAttitudeRate() const;
  const arlcore::CovarAttitudeRateType& getAttitudeRateCovariance() const;
  const arlcore::Velocity3DPlatformNEDType& getVelocity() const;
  const arlcore::CovarianceVelocityType& getVelocityCovariance() const;

  const flt64_t& getPitchRate() const;
  const flt64_t& getRollRate() const;
  const flt64_t& getYawRate() const;

  const flt64_t& getDownSpeed() const;
  const flt64_t& getEastSpeed() const;
  const flt64_t& getNorthSpeed() const;

  void setAttitudeRate(const flt64_t& inPitchRate,
            const flt64_t& InRollRate,
            const flt64_t& InYawRate);

  void setAttitudeRateCovariance(const flt64_t& inPitchPitchAttRateCovar,
            const flt64_t& inPitchYawAttRateCovar,
            const flt64_t& inRollPitchAttRateCovar,
            const flt64_t& inRollRollAttRateCovar,
            const flt64_t& inRollYawAttRateCovar,
            const flt64_t& inYawYawAttRateCovar);

  void setVelocity(const flt64_t& inDownSpeed,
            const flt64_t& InEastSpeed,
            const flt64_t& NorthSpeed);

  void setVelocityCovariance(const flt64_t& inDownDownVelErrCovar,
            const flt64_t& inEastDownVelErrCovar,
            const flt64_t& inEastEastVelErrCovar,
            const flt64_t& inNorthDownVelErrCovar,
            const flt64_t& inNorthEastVelErrCovar,
            const flt64_t& inNorthNorthVelErrCovar);

 private:
  arlcore::OrientationVel3D attitudeRate_;
  arlcore::CovarAttitudeRateType attitudeRateCovariance_;
  arlcore::Velocity3DPlatformNEDType velocity_;
  arlcore::CovarianceVelocityType velocityCovariance_;
};

}  // namespace arlcore

#endif  // INCLUDE_DOMAIN_VELOCITYSTATUSDATA_H_
