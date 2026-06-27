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

#include "VelocityStatusData.h"

namespace arlcore {
  const arlcore::OrientationVel3D& VelocityStatusData::getAttitudeRate() const {
    return attitudeRate_;
  }
  const arlcore::CovarAttitudeRateType&
    VelocityStatusData::getAttitudeRateCovariance() const {
    return attitudeRateCovariance_;
  }
  const arlcore::Velocity3DPlatformNEDType&
    VelocityStatusData::getVelocity() const {
    return velocity_;
  }
  const arlcore::CovarianceVelocityType&
    VelocityStatusData::getVelocityCovariance() const {
    return velocityCovariance_;
  }

  const flt64_t& VelocityStatusData::getPitchRate() const {
    return attitudeRate_.getPitchRate();
  }
  const flt64_t& VelocityStatusData::getRollRate() const {
    return attitudeRate_.getRollRate();
  }
  const flt64_t& VelocityStatusData::getYawRate() const {
    return attitudeRate_.getYawRate();
  }

  const flt64_t& VelocityStatusData::getDownSpeed() const {
    return velocity_.getDownSpeed();
  }
  const flt64_t& VelocityStatusData::getEastSpeed() const {
    return velocity_.getEastSpeed();
  }
  const flt64_t& VelocityStatusData::getNorthSpeed() const {
    return velocity_.getNorthSpeed();
  }

  void VelocityStatusData::setAttitudeRate(const flt64_t& inPitchRate,
            const flt64_t& InRollRate,
            const flt64_t& InYawRate) {
    attitudeRate_.setPitchRate(inPitchRate);
    attitudeRate_.setRollRate(InRollRate);
    attitudeRate_.setYawRate(InYawRate);
  }

  void VelocityStatusData::setAttitudeRateCovariance(
            const flt64_t& inPitchPitchAttRateCovar,
            const flt64_t& inPitchYawAttRateCovar,
            const flt64_t& inRollPitchAttRateCovar,
            const flt64_t& inRollRollAttRateCovar,
            const flt64_t& inRollYawAttRateCovar,
            const flt64_t& inYawYawAttRateCovar) {
    attitudeRateCovariance_.setPitchPitchAttRateErrCovar(
        inPitchPitchAttRateCovar);
    attitudeRateCovariance_.setPitchYawAttRateErrCovar(inPitchYawAttRateCovar);
    attitudeRateCovariance_.setRollPitchAttRateErrCovar(
        inRollPitchAttRateCovar);
    attitudeRateCovariance_.setRollRollAttRateErrCovar(inRollRollAttRateCovar);
    attitudeRateCovariance_.setRollYawAttRateErrCovar(inRollYawAttRateCovar);
    attitudeRateCovariance_.setYawYawAttRateErrCovar(inYawYawAttRateCovar);
  }

  void VelocityStatusData::setVelocity(const flt64_t& inDownSpeed,
            const flt64_t& InEastSpeed,
            const flt64_t& NorthSpeed) {
    velocity_.setDownSpeed(inDownSpeed);
    velocity_.setEastSpeed(InEastSpeed);
    velocity_.setNorthSpeed(NorthSpeed);
  }

  void VelocityStatusData::setVelocityCovariance(
            const flt64_t& inDownDownVelErrCovar,
            const flt64_t& inEastDownVelErrCovar,
            const flt64_t& inEastEastVelErrCovar,
            const flt64_t& inNorthDownVelErrCovar,
            const flt64_t& inNorthEastVelErrCovar,
            const flt64_t& inNorthNorthVelErrCovar) {
    velocityCovariance_.setDownDownVelErrCovar(inDownDownVelErrCovar);
    velocityCovariance_.setEastDownVelErrCovar(inEastDownVelErrCovar);
    velocityCovariance_.setEastEastVelErrCovar(inEastEastVelErrCovar);
    velocityCovariance_.setNorthDownVelErrCovar(inNorthDownVelErrCovar);
    velocityCovariance_.setNorthEastVelErrCovar(inNorthEastVelErrCovar);
    velocityCovariance_.setNorthNorthVelErrCovar(inNorthNorthVelErrCovar);
  }

}  // namespace arlcore
