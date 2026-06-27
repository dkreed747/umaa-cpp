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

#include <string>

#include "GlobalPoseData.h"
#include "Logger.h"

namespace arlcore {

const flt64_t& GlobalPoseData::getPitch() const {
  return gpAtt_.getPitch();
}

const flt64_t& GlobalPoseData::getRoll() const {
  return gpAtt_.getRoll();
}

const flt64_t& GlobalPoseData::getYaw() const {
  return gpAtt_.getYaw();
}

const flt64_t& GlobalPoseData::getLatitude() const {
  return latitude_;
}

const flt64_t& GlobalPoseData::getLongitude() const {
  return longitude_;
}

const flt64_t& GlobalPoseData::getAltitude() const {
  return altitude_;
}
const flt64_t& GlobalPoseData::getAltitudeAGL() const {
  return altitudeAGL_;
}
const flt64_t& GlobalPoseData::getAltitudeASF() const {
  return altitudeASF_;
}
const flt64_t& GlobalPoseData::getAltitudeGeodetic() const {
  return altitudeGeodetic_;
}
const flt64_t& GlobalPoseData::getCourse() const {
  return course_;
}
const flt64_t& GlobalPoseData::getDepth() const {
  return depth_;
}

flt64_t GlobalPoseData::getElevationFromTypeSpecifier(const Elevations& specifier) const {
  flt64_t retVal = -1.0;
  switch (specifier) {
    case ABOVE_GROUND_LEVEL:
      retVal = altitudeAGL_;
      break;
    case ABOVE_SEA_FLOOR:
      retVal = altitudeASF_;
      break;
    case GEODETIC:
      retVal = altitudeGeodetic_;
      break;
    case MEAN_SEA_LEVEL:
      retVal = altitude_;
      break;
    case DEPTH_BELOW_SEA_LEVEL:
      retVal = depth_;
      break;
    default:
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER,
        "Elevation type specifier (" << specifier << ") Not supported")
  }
  return retVal;
}

flt64_t GlobalPoseData::getDirectionFromTypeSpecifier(const Directions& specifier) const {
  flt64_t retVal = -1.0;
  switch (specifier) {
    case CURRENT:
      // Convert YawZNEDType to be relative to current direction
      retVal = getAttitude().getYaw();
      break;
    case MAGNETIC_NORTH:
      // Convert YawZNEDType to be relative to magnetic north
      retVal = getAttitude().getYaw();
      break;
    case TRUE_NORTH:
      // Convert YawZNEDType to be relative to true north
      retVal = getAttitude().getYaw();
      break;
    case WIND:
      // Convert YawZNEDType to be relative to wind
      retVal = getAttitude().getYaw();
      break;
    default:
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER,
        "Direction type specifier (" << specifier << ") Not supported")
  }
  return retVal;
}

const flt64_t& GlobalPoseData::getPitchRate() const {
  return vsData_.getPitchRate();
}
const flt64_t& GlobalPoseData::getRollRate() const {
  return vsData_.getRollRate();
}
const flt64_t& GlobalPoseData::getYawRate() const {
  return vsData_.getYawRate();
}
const flt64_t& GlobalPoseData::getDownSpeed() const {
  return vsData_.getDownSpeed();
}
const flt64_t& GlobalPoseData::getEastSpeed() const {
  return vsData_.getEastSpeed();
}
const flt64_t& GlobalPoseData::getNorthSpeed() const {
  return vsData_.getNorthSpeed();
}

const arlcore::OrientationVel3D& GlobalPoseData::getAttitudeRate() const {
  return vsData_.getAttitudeRate();
}
const arlcore::CovarianceOrientationType&
  GlobalPoseData::getAttitudeCovariance() const {
  return attitudeCovariance_;
}
const arlcore::CovariancePositionNEDType&
  GlobalPoseData::getPositionCovariance() const {
  return positionCovariance_;
}

const arlcore::CovarAttitudeRateType&
  GlobalPoseData::getAttitudeRateCovariance() const {
  return vsData_.getAttitudeRateCovariance();
}
const arlcore::Velocity3DPlatformNEDType&
  GlobalPoseData::getVelocity() const {
  return vsData_.getVelocity();
}
const arlcore::CovarianceVelocityType&
  GlobalPoseData::getVelocityCovariance() const {
  return vsData_.getVelocityCovariance();
}

void GlobalPoseData::setAttitude(const flt64_t& inPitch,
          const flt64_t& inRoll,
          const flt64_t& inYaw) {
  gpAtt_.setPitch(inPitch);
  gpAtt_.setRoll(inRoll);
  gpAtt_.setYaw(inYaw);
}

const GlobalPoseAttitude& GlobalPoseData::getAttitude() const {
  return gpAtt_;
}

void GlobalPoseData::setLatitude(const flt64_t& inLatitude) {
  latitude_ = inLatitude;
}

void GlobalPoseData::setLongitude(const flt64_t& inLongitude) {
  longitude_ = inLongitude;
}

void GlobalPoseData::setAltitude(const flt64_t& inAltitude) {
  altitude_ = inAltitude;
}
void GlobalPoseData::setAltitudeAGL(const flt64_t& inAltitudeAGL) {
  altitudeAGL_ = inAltitudeAGL;
}
void GlobalPoseData::setAltitudeASF(const flt64_t& inAltitudeASF) {
  altitudeASF_ = inAltitudeASF;
}
void GlobalPoseData::setAltitudeGeodetic(const flt64_t& inAltitudeGeodetic) {
  altitudeGeodetic_ = inAltitudeGeodetic;
}
void GlobalPoseData::setCourse(const flt64_t& inCourse) {
  course_ = inCourse;
}
void GlobalPoseData::setDepth(const flt64_t& inDepth) {
  depth_ = inDepth;
}

void GlobalPoseData::setAttitudeRate(const flt64_t& inPitchRate,
          const flt64_t& inRollRate,
          const flt64_t& inYawRate) {
  vsData_.setAttitudeRate(inPitchRate, inRollRate, inYawRate);
}

void GlobalPoseData::setAttitudeRateCovariance(
          const flt64_t& inPitchPitchAttRateCovar,
          const flt64_t& inPitchYawAttRateCovar,
          const flt64_t& inRollPitchAttRateCovar,
          const flt64_t& inRollRollAttRateCovar,
          const flt64_t& inRollYawAttRateCovar,
          const flt64_t& inYawYawAttRateCovar) {
  vsData_.setAttitudeRateCovariance(inPitchPitchAttRateCovar,
      inPitchYawAttRateCovar,
      inRollPitchAttRateCovar,
      inRollRollAttRateCovar,
      inRollYawAttRateCovar,
      inYawYawAttRateCovar);
}

void GlobalPoseData::setVelocity(const flt64_t& inDownSpeed,
          const flt64_t& inEastSpeed,
          const flt64_t& inNorthSpeed) {
  vsData_.setVelocity(inDownSpeed, inEastSpeed, inNorthSpeed);
}

void GlobalPoseData::setVelocityCovariance(
          const flt64_t& inDownDownVelErrCovar,
          const flt64_t& inEastDownVelErrCovar,
          const flt64_t& inEastEastVelErrCovar,
          const flt64_t& inNorthDownVelErrCovar,
          const flt64_t& inNorthEastVelErrCovar,
          const flt64_t& inNorthNorthVelErrCovar) {
  vsData_.setVelocityCovariance(inDownDownVelErrCovar,
      inEastDownVelErrCovar,
      inEastEastVelErrCovar,
      inNorthDownVelErrCovar,
      inNorthEastVelErrCovar,
      inNorthNorthVelErrCovar);
}

void GlobalPoseData::setPositionCovariance(const flt64_t& inDownDownPosErr,
            const flt64_t& inEastDownPosErr,
            const flt64_t& inEastEastPosErr,
            const flt64_t& inNorthDownPosErr,
            const flt64_t& inNorthEastPosErr,
            const flt64_t& inNorthNorthPosErr) {
  positionCovariance_.setDownDownPosErrCovar(inDownDownPosErr);
  positionCovariance_.setEastDownPosErrCovar(inEastDownPosErr);
  positionCovariance_.setEastEastPosErrCovar(inEastEastPosErr);
  positionCovariance_.setNorthDownPosErrCovar(inNorthDownPosErr);
  positionCovariance_.setNorthEastPosErrCovar(inNorthEastPosErr);
  positionCovariance_.setNorthNorthPosErrCovar(inNorthNorthPosErr);
}

void GlobalPoseData::setAttitudeCovariance(const flt64_t& inPitchPitchAngleErr,
            const flt64_t& inPitchYawAngleErr,
            const flt64_t& inRollPitchAngleErr,
            const flt64_t& inRollRollAngleErr,
            const flt64_t& inRollYawAngleErr,
            const flt64_t& inYawYawAngleErr) {
  attitudeCovariance_.setPitchPitchAngleErrCovar(inPitchPitchAngleErr);
  attitudeCovariance_.setPitchYawAngleErrCovar(inPitchYawAngleErr);
  attitudeCovariance_.setRollPitchAngleErrCovar(inRollPitchAngleErr);
  attitudeCovariance_.setRollRollAngleErrCovar(inRollRollAngleErr);
  attitudeCovariance_.setRollYawAngleErrCovar(inRollYawAngleErr);
  attitudeCovariance_.setYawYawAngleErrCovar(inYawYawAngleErr);
}

}  // namespace arlcore
