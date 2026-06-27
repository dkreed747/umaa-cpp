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
#include <gtest/gtest.h>

#include "GlobalPoseData.h"
#include "EnumSpecifiers.h"

TEST(GlobalPoseDataTest, setAttitude) {
	arlcore::GlobalPoseData gpData;

	flt64_t expPitch = 100.1;
	flt64_t expRoll = 98.2;
	flt64_t expYaw = 96.3;

	gpData.setAttitude(expPitch, expRoll, expYaw);

	EXPECT_EQ(expPitch, gpData.getAttitude().getPitch());
	EXPECT_EQ(expRoll, gpData.getAttitude().getRoll());
	EXPECT_EQ(expYaw, gpData.getAttitude().getYaw());

}

TEST(GlobalPoseDataTest, setLongitude) {
	arlcore::GlobalPoseData gpData;

	flt64_t expVal = 10.0;

	gpData.setLongitude(expVal);
	EXPECT_EQ(expVal, gpData.getLongitude());
}

TEST(GlobalPoseDataTest, setLatitude) {
	arlcore::GlobalPoseData gpData;

	flt64_t expVal = 1.0;

	gpData.setLatitude(expVal);
	EXPECT_EQ(expVal, gpData.getLatitude());
}

TEST(GlobalPoseDataTest, setAltitude) {
  arlcore::GlobalPoseData gpData;

  flt64_t expAlt = 100.1;

  gpData.setAltitude(expAlt);

  EXPECT_EQ(expAlt, gpData.getAltitude());
}

TEST(GlobalPoseDataTest, setAltitudeAGL) {
  arlcore::GlobalPoseData gpData;

  flt64_t expAlt = 10.1;

  gpData.setAltitudeAGL(expAlt);

  EXPECT_EQ(expAlt, gpData.getAltitudeAGL());
}

TEST(GlobalPoseDataTest, setAltitudeASF) {
  arlcore::GlobalPoseData gpData;

  flt64_t expAlt = 20000.1;

  gpData.setAltitudeASF(expAlt);

  EXPECT_EQ(expAlt, gpData.getAltitudeASF());
}

TEST(GlobalPoseDataTest, setAltitudeGeodetic) {
  arlcore::GlobalPoseData gpData;

  flt64_t expAlt = -1500.1;

  gpData.setAltitudeGeodetic(expAlt);

  EXPECT_EQ(expAlt, gpData.getAltitudeGeodetic());
}

TEST(GlobalPoseDataTest, setCourse) {
  arlcore::GlobalPoseData gpData;

  flt64_t expCourse = 3.14;

  gpData.setCourse(expCourse);

  EXPECT_EQ(expCourse, gpData.getCourse());
}

TEST(GlobalPoseDataTest, setDepth) {
  arlcore::GlobalPoseData gpData;

  flt64_t expDepth = 9000.1;

  gpData.setDepth(expDepth);

  EXPECT_EQ(expDepth, gpData.getDepth());
}

TEST(GlobalPoseDataTest, setVelocity) {
  arlcore::GlobalPoseData gpData;

  flt64_t expDownSpeed = 100000.1;
  flt64_t expEastSpeed = -25.23;
  flt64_t expNorthSpeed = 1234.56;

  gpData.setVelocity(expDownSpeed, expEastSpeed, expNorthSpeed);

  EXPECT_EQ(expDownSpeed, gpData.getDownSpeed());
  EXPECT_EQ(expEastSpeed, gpData.getEastSpeed());
  EXPECT_EQ(expNorthSpeed, gpData.getNorthSpeed());

  const arlcore::Velocity3DPlatformNEDType expResult = gpData.getVelocity();

  EXPECT_EQ(expDownSpeed, expResult.getDownSpeed());
  EXPECT_EQ(expEastSpeed, expResult.getEastSpeed());
  EXPECT_EQ(expNorthSpeed, expResult.getNorthSpeed());

}


TEST(GlobalPoseDataTest, setAttitudeRate) {
  arlcore::GlobalPoseData gpData;

  flt64_t expPitchRate = 15.1;
  flt64_t expRollRate = 50.23;
  flt64_t expYawRate = -32.32;

  gpData.setAttitudeRate(expPitchRate, expRollRate, expYawRate);

  EXPECT_EQ(expPitchRate, gpData.getPitchRate());
  EXPECT_EQ(expRollRate, gpData.getRollRate());
  EXPECT_EQ(expYawRate, gpData.getYawRate());

  const arlcore::OrientationVel3D expRates = gpData.getAttitudeRate();

  EXPECT_EQ(expPitchRate, expRates.getPitchRate());
  EXPECT_EQ(expRollRate, expRates.getRollRate());
  EXPECT_EQ(expYawRate, expRates.getYawRate());

}

TEST(GlobalPoseDataTest, setVelocityCovar) {
  arlcore::GlobalPoseData gpData;

  flt64_t expDownDownVelErr = 15.1;
  flt64_t expEastDownVelErr= 50.23;
  flt64_t expEastEastVelErr = -32.32;
  flt64_t expNorthDownVelErr = 150.1;
  flt64_t expNorthEastVelErr= 500.23;
  flt64_t expNorthNorthVelErr = -320.32;

  gpData.setVelocityCovariance(expDownDownVelErr, expEastDownVelErr, expEastEastVelErr,
      expNorthDownVelErr, expNorthEastVelErr, expNorthNorthVelErr);

  const arlcore::CovarianceVelocityType compositeResult =  gpData.getVelocityCovariance();

  EXPECT_EQ(expDownDownVelErr, compositeResult.getDownDownVelErrCovar());
  EXPECT_EQ(expEastDownVelErr, compositeResult.getEastDownVelErrCovar());
  EXPECT_EQ(expEastEastVelErr, compositeResult.getEastEastVelErrCovar());
  EXPECT_EQ(expNorthDownVelErr, compositeResult.getNorthDownVelErrCovar());
  EXPECT_EQ(expNorthEastVelErr, compositeResult.getNorthEastVelErrCovar());
  EXPECT_EQ(expNorthNorthVelErr, compositeResult.getNorthNorthVelErrCovar());

}

TEST(GlobalPoseDataTest, setAttitudeRateCovar) {
  flt64_t expPitchPitchRateErr = 15.1;
  flt64_t expPitchYawRateErr= 50.23;
  flt64_t expRollPitchRateErr = -32.32;
  flt64_t expRollRollRateErr = 150.1;
  flt64_t expRollYawRateErr= 500.23;
  flt64_t expYawYawRateErr = -320.32;

  arlcore::CovarAttitudeRateType compositeResult;

  compositeResult.setPitchPitchAttRateErrCovar(expPitchPitchRateErr);
  compositeResult.setPitchYawAttRateErrCovar(expPitchYawRateErr);
  compositeResult.setRollPitchAttRateErrCovar(expRollPitchRateErr);
  compositeResult.setRollRollAttRateErrCovar(expRollRollRateErr);
  compositeResult.setRollYawAttRateErrCovar(expRollYawRateErr);
  compositeResult.setYawYawAttRateErrCovar(expYawYawRateErr);

  EXPECT_EQ(expPitchPitchRateErr, compositeResult.getPitchPitchAttRateErrCovar());
  EXPECT_EQ(expPitchYawRateErr, compositeResult.getPitchYawAttRateErrCovar());
  EXPECT_EQ(expRollPitchRateErr, compositeResult.getRollPitchAttRateErrCovar());
  EXPECT_EQ(expRollRollRateErr, compositeResult.getRollRollAttRateErrCovar());
  EXPECT_EQ(expRollYawRateErr, compositeResult.getRollYawAttRateErrCovar());
  EXPECT_EQ(expYawYawRateErr, compositeResult.getYawYawAttRateErrCovar());
}

TEST(GlobalPoseDataTest, setAttitudeRateCovarMain) {
  flt64_t expPitchPitchRateErr = 15.1;
  flt64_t expPitchYawRateErr= 50.23;
  flt64_t expRollPitchRateErr = -32.32;
  flt64_t expRollRollRateErr = 150.1;
  flt64_t expRollYawRateErr= 500.23;
  flt64_t expYawYawRateErr = -320.32;
  arlcore::GlobalPoseData gpData;

  gpData.setAttitudeRateCovariance(expPitchPitchRateErr,
    expPitchYawRateErr, expRollPitchRateErr, expRollRollRateErr,
    expRollYawRateErr, expYawYawRateErr);

  EXPECT_EQ(expPitchPitchRateErr, gpData.getAttitudeRateCovariance().getPitchPitchAttRateErrCovar());
  EXPECT_EQ(expPitchYawRateErr, gpData.getAttitudeRateCovariance().getPitchYawAttRateErrCovar());
  EXPECT_EQ(expRollPitchRateErr, gpData.getAttitudeRateCovariance().getRollPitchAttRateErrCovar());
  EXPECT_EQ(expRollRollRateErr, gpData.getAttitudeRateCovariance().getRollRollAttRateErrCovar());
  EXPECT_EQ(expRollYawRateErr, gpData.getAttitudeRateCovariance().getRollYawAttRateErrCovar());
  EXPECT_EQ(expYawYawRateErr, gpData.getAttitudeRateCovariance().getYawYawAttRateErrCovar());
}

TEST(GlobalPoseDataTest, setCompositeVelocityCovar) {
  flt64_t expDownDownVelErr = 15.1;
  flt64_t expEastDownVelErr= 50.23;
  flt64_t expEastEastVelErr = -32.32;
  flt64_t expNorthDownVelErr = 150.1;
  flt64_t expNorthEastVelErr= 500.23;
  flt64_t expNorthNorthVelErr = -320.32;

  arlcore::CovarianceVelocityType compositeResult;

  compositeResult.setDownDownVelErrCovar(expDownDownVelErr);
  compositeResult.setEastDownVelErrCovar(expEastDownVelErr);
  compositeResult.setEastEastVelErrCovar(expEastEastVelErr);
  compositeResult.setNorthDownVelErrCovar(expNorthDownVelErr);
  compositeResult.setNorthEastVelErrCovar(expNorthEastVelErr);
  compositeResult.setNorthNorthVelErrCovar(expNorthNorthVelErr);

  EXPECT_EQ(expDownDownVelErr, compositeResult.getDownDownVelErrCovar());
  EXPECT_EQ(expEastDownVelErr, compositeResult.getEastDownVelErrCovar());
  EXPECT_EQ(expEastEastVelErr, compositeResult.getEastEastVelErrCovar());
  EXPECT_EQ(expNorthDownVelErr, compositeResult.getNorthDownVelErrCovar());
  EXPECT_EQ(expNorthEastVelErr, compositeResult.getNorthEastVelErrCovar());
  EXPECT_EQ(expNorthNorthVelErr, compositeResult.getNorthNorthVelErrCovar());
}

TEST(GlobalPoseDataTest, setOrientationVel3D) {
  flt64_t expPitchRate = 15.1;
  flt64_t expRollRate = 50.23;
  flt64_t expYawRate = -32.32;

  arlcore::OrientationVel3D compositeResult;

  compositeResult.setPitchRate(expPitchRate);
  compositeResult.setRollRate(expRollRate);
  compositeResult.setYawRate(expYawRate);

  EXPECT_EQ(expPitchRate, compositeResult.getPitchRate());
  EXPECT_EQ(expRollRate, compositeResult.getRollRate());
  EXPECT_EQ(expYawRate, compositeResult.getYawRate());
}

TEST(GlobalPoseDataTest, setPositionCovar) {
  arlcore::GlobalPoseData gpData;

  flt64_t expDownDownPosErr = 15.1;
  flt64_t expEastDownPosErr= 50.23;
  flt64_t expEastEastPosErr = -32.32;
  flt64_t expNorthDownPosErr = 150.1;
  flt64_t expNorthEastPosErr= 500.23;
  flt64_t expNorthNorthPosErr = -320.32;

  gpData.setPositionCovariance(expDownDownPosErr, expEastDownPosErr, expEastEastPosErr,
      expNorthDownPosErr, expNorthEastPosErr, expNorthNorthPosErr);

  EXPECT_EQ(expDownDownPosErr, gpData.getPositionCovariance().getDownDownPosErrCovar());
  EXPECT_EQ(expEastDownPosErr, gpData.getPositionCovariance().getEastDownPosErrCovar());
  EXPECT_EQ(expEastEastPosErr, gpData.getPositionCovariance().getEastEastPosErrCovar());
  EXPECT_EQ(expNorthDownPosErr, gpData.getPositionCovariance().getNorthDownPosErrCovar());
  EXPECT_EQ(expNorthEastPosErr, gpData.getPositionCovariance().getNorthEastPosErrCovar());
  EXPECT_EQ(expNorthNorthPosErr, gpData.getPositionCovariance().getNorthNorthPosErrCovar());
}

TEST(GlobalPoseDataTest, setAttitudeCovar) {
  arlcore::GlobalPoseData gpData;

  flt64_t expPitchPitchAngleErr = 15.1;
  flt64_t expPitchYawAngleErr= 50.23;
  flt64_t expRollPitchAngleErr = -32.32;
  flt64_t expRollRollAngleErr = 150.1;
  flt64_t expRollYawAngleErr= 500.23;
  flt64_t expYawYawAngleErr = -320.32;

  gpData.setAttitudeCovariance(expPitchPitchAngleErr,
      expPitchYawAngleErr,
      expRollPitchAngleErr,
      expRollRollAngleErr,
      expRollYawAngleErr,
      expYawYawAngleErr);

  EXPECT_EQ(expPitchPitchAngleErr, gpData.getAttitudeCovariance().getPitchPitchAngleErrCovar());
  EXPECT_EQ(expPitchYawAngleErr, gpData.getAttitudeCovariance().getPitchYawAngleErrCovar());
  EXPECT_EQ(expRollPitchAngleErr, gpData.getAttitudeCovariance().getRollPitchAngleErrCovar());
  EXPECT_EQ(expRollRollAngleErr, gpData.getAttitudeCovariance().getRollRollAngleErrCovar());
  EXPECT_EQ(expRollYawAngleErr, gpData.getAttitudeCovariance().getRollYawAngleErrCovar());
  EXPECT_EQ(expYawYawAngleErr, gpData.getAttitudeCovariance().getYawYawAngleErrCovar());
}

TEST(GlobalPoseDataTest, getElevationFromSpecifier) {
  arlcore::GlobalPoseData gpData;

  flt64_t AGL = 12.62;
  flt64_t ASF = 7.58;
  flt64_t GEO = 10.28;
  flt64_t MSL = 39.74;
  flt64_t DEP = 17.17;
  gpData.setAltitudeAGL(AGL);
  gpData.setAltitudeASF(ASF);
  gpData.setAltitudeGeodetic(GEO);
  gpData.setAltitude(MSL);
  gpData.setDepth(DEP);

  EXPECT_EQ(gpData.getElevationFromTypeSpecifier(arlcore::Elevations::ABOVE_GROUND_LEVEL), AGL);
  EXPECT_EQ(gpData.getElevationFromTypeSpecifier(arlcore::Elevations::ABOVE_SEA_FLOOR), ASF);
  EXPECT_EQ(gpData.getElevationFromTypeSpecifier(arlcore::Elevations::GEODETIC), GEO);
  EXPECT_EQ(gpData.getElevationFromTypeSpecifier(arlcore::Elevations::MEAN_SEA_LEVEL), MSL);
  EXPECT_EQ(gpData.getElevationFromTypeSpecifier(arlcore::Elevations::DEPTH_BELOW_SEA_LEVEL), DEP);

  // Force bad elevation type to test the catch all
  EXPECT_EQ(gpData.getElevationFromTypeSpecifier(static_cast<arlcore::Elevations>(7)), -1.0);
}

TEST(GlobalPoseDataTest, getDirectionFromSpecifier) {
  arlcore::GlobalPoseData gpData;

  flt64_t pitch = 40.56;
  flt64_t roll = 28.55;
  flt64_t yaw = 36.38;

  gpData.setAttitude(pitch, roll, yaw);

  // Command is assumed to be in NED system so no conversion is necessary now
  // if this changes, modify the getDirectionFromTypeSpecifier function to
  // augment behavior

  EXPECT_EQ(gpData.getDirectionFromTypeSpecifier(arlcore::Directions::CURRENT), yaw);
  EXPECT_EQ(gpData.getDirectionFromTypeSpecifier(arlcore::Directions::MAGNETIC_NORTH), yaw);
  EXPECT_EQ(gpData.getDirectionFromTypeSpecifier(arlcore::Directions::TRUE_NORTH), yaw);
  EXPECT_EQ(gpData.getDirectionFromTypeSpecifier(arlcore::Directions::WIND), yaw);
}


