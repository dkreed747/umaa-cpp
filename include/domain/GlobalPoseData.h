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

#ifndef INCLUDE_DOMAIN_GLOBALPOSEDATA_H_
#define INCLUDE_DOMAIN_GLOBALPOSEDATA_H_

#include "InternalTypes.h"
#include "GlobalPoseAttitude.h"
#include "VelocityStatusData.h"
#include "CovarianceOrientationType.h"
#include "CovariancePositionNEDType.h"
#include "EnumSpecifiers.h"

namespace arlcore {

//! \brief Common Global Position Data class containing
class GlobalPoseData {
 public:
  GlobalPoseData() = default;
  virtual ~GlobalPoseData() = default;

  const flt64_t& getPitch() const;
  const flt64_t& getRoll() const;
  const flt64_t& getYaw() const;

  bool operator==(const GlobalPoseData& lhs) const {
    return (latitude_ == lhs.latitude_
          && longitude_ == lhs.longitude_);
  }

  bool operator!=(const GlobalPoseData& lhs) const {
    return !(*this == lhs);
  }

  const flt64_t& getLatitude() const;
  const flt64_t& getLongitude() const;

  const flt64_t& getPitchRate() const;
  const flt64_t& getRollRate() const;
  const flt64_t& getYawRate() const;
  const flt64_t& getDownSpeed() const;
  const flt64_t& getEastSpeed() const;
  const flt64_t& getNorthSpeed() const;

  const flt64_t& getAltitude() const;
  const flt64_t& getAltitudeAGL() const;
  const flt64_t& getAltitudeASF() const;
  const flt64_t& getAltitudeGeodetic() const;
  const flt64_t& getCourse() const;
  const flt64_t& getDepth() const;

  flt64_t getElevationFromTypeSpecifier(const Elevations& specifier) const;
  flt64_t getDirectionFromTypeSpecifier(const Directions& specifier) const;

  const GlobalPoseAttitude& getAttitude() const;
  const CovarianceOrientationType& getAttitudeCovariance() const;
  const CovariancePositionNEDType& getPositionCovariance() const;

  const arlcore::OrientationVel3D& getAttitudeRate() const;

  const arlcore::Velocity3DPlatformNEDType& getVelocity() const;

  const arlcore::CovarianceVelocityType& getVelocityCovariance() const;
  const arlcore::CovarAttitudeRateType& getAttitudeRateCovariance() const;

  void setAltitude(const flt64_t& inAltitude);
  void setAltitudeAGL(const flt64_t& inAltitudeAGL);
  void setAltitudeASF(const flt64_t& inAltitudeASF);
  void setAltitudeGeodetic(const flt64_t& inAltitudeGeodetic);
  void setCourse(const flt64_t& inCourse);
  void setDepth(const flt64_t& inDepth);

  void setAttitude(const flt64_t& inPitch,
            const flt64_t& inRoll,
            const flt64_t& inYaw);

  void setLatitude(const flt64_t& inLatitude);
  void setLongitude(const flt64_t& inLongitude);

  // Velocity Status Attitude Rate values
  void setAttitudeRate(const flt64_t& inPitchRate,
            const flt64_t& inRollRate,
            const flt64_t& inYawRate);

  // Velocity Status Attitude Rate Covariance values
  void setAttitudeRateCovariance(const flt64_t& inPitchPitchAttRateCovar,
            const flt64_t& inPitchYawAttRateCovar,
            const flt64_t& inRollPitchAttRateCovar,
            const flt64_t& inRollRollAttRateCovar,
            const flt64_t& inRollYawAttRateCovar,
            const flt64_t& inYawYawAttRateCovar);

  // Velocity Status Velocity values
  void setVelocity(const flt64_t& inDownSpeed,
            const flt64_t& inEastSpeed,
            const flt64_t& inNorthSpeed);

  // Velocity Status Velocity Covariance values
  void setVelocityCovariance(const flt64_t& inDownDownVelErrCovar,
            const flt64_t& inEastDownVelErrCovar,
            const flt64_t& inEastEastVelErrCovar,
            const flt64_t& inNorthDownVelErrCovar,
            const flt64_t& inNorthEastVelErrCovar,
            const flt64_t& inNorthNorthVelErrCovar);

  // Global Pose Position Covariance values
  void setPositionCovariance(const flt64_t& inDownDownPosErr,
            const flt64_t& inEastDownPosErr,
            const flt64_t& inEastEastPosErr,
            const flt64_t& inNorthDownPosErr,
            const flt64_t& inNorthEastPosErr,
            const flt64_t& inNorthNorthPosErr);

  // Global Pose Attitude Covariance values
  void setAttitudeCovariance(const flt64_t& inPitchPitchAngleErr,
            const flt64_t& inPitchYawAngleErr,
            const flt64_t& inRollPitchAngleErr,
            const flt64_t& inRollRollAngleErr,
            const flt64_t& inRollYawAngleErr,
            const flt64_t& inYawYawAngleErr);

 private:
  arlcore::GlobalPoseAttitude gpAtt_;
  arlcore::VelocityStatusData vsData_;
  arlcore::CovarianceOrientationType attitudeCovariance_;
  arlcore::CovariancePositionNEDType positionCovariance_;
  flt64_t latitude_ = 0.0;
  flt64_t longitude_ = 0.0;
  flt64_t altitude_ = 0.0;
  flt64_t altitudeAGL_ = 0.0;
  flt64_t altitudeASF_ = 0.0;
  flt64_t altitudeGeodetic_ = 0.0;
  flt64_t course_ = 0.0;
  flt64_t depth_ = 0.0;
};

}  // namespace arlcore

#endif  // INCLUDE_DOMAIN_GLOBALPOSEDATA_H_
