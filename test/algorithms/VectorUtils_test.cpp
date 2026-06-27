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

#include "VectorUtils.h"

using UMAA::MO::GlobalVectorControl::GlobalVectorCommandType;

GlobalVectorCommandType getNominalGvCommand() {
  GlobalVectorCommandType gvCmd;

  gvCmd.directionMode(UMAA::Common::MaritimeEnumeration::DirectionModeEnumModule::DirectionModeEnumType::COURSE);
  gvCmd.direction().DirectionRequirementVariantTypeSubtypes().DirectionTrueNorthRequirementVariantVariant(UMAA::Common::Orientation::DirectionTrueNorthRequirementVariantType());
  gvCmd.direction().DirectionRequirementVariantTypeSubtypes().DirectionTrueNorthRequirementVariantVariant().direction().direction(0);

  UMAA::Common::Speed::SpeedRequirementVariantType speedType;
  speedType.SpeedRequirementVariantTypeSubtypes().GroundSpeedRequirementVariantVariant(UMAA::Common::Speed::GroundSpeedRequirementVariantType());
  speedType.SpeedRequirementVariantTypeSubtypes().GroundSpeedRequirementVariantVariant().speed().speed(0);

  gvCmd.speed(speedType);

  // Set default elevation requirement to depth.
  auto elevationEnum = UMAA::Common::Measurement::ElevationRequirementVariantTypeEnum::DEPTHREQUIREMENTVARIANT_D;
  UMAA::Common::Measurement::ElevationRequirementVariantType elevationReq;
  elevationReq.ElevationRequirementVariantTypeSubtypes().DepthRequirementVariantVariant(UMAA::Common::Measurement::DepthRequirementVariantType());
  elevationReq.ElevationRequirementVariantTypeSubtypes().DepthRequirementVariantVariant().depth().depth(0);

  gvCmd.elevation(elevationReq);

  return gvCmd;
}


GlobalVectorCommandType generateElevationVector(
    UMAA::Common::Measurement::ElevationRequirementVariantTypeEnum elevationEnum, flt64_t elevationValue ) {
  GlobalVectorCommandType vector = getNominalGvCommand();

  UMAA::Common::Measurement::ElevationRequirementVariantType elevationReq;

  switch (elevationEnum) {
    case UMAA::Common::Measurement::ElevationRequirementVariantTypeEnum::ALTITUDEAGLREQUIREMENTVARIANT_D:
      elevationReq.ElevationRequirementVariantTypeSubtypes().AltitudeAGLRequirementVariantVariant(UMAA::Common::Measurement::AltitudeAGLRequirementVariantType());
      elevationReq.ElevationRequirementVariantTypeSubtypes().AltitudeAGLRequirementVariantVariant().altitude().altitude(elevationValue);
      break;
    case UMAA::Common::Measurement::ElevationRequirementVariantTypeEnum::ALTITUDEASFREQUIREMENTVARIANT_D:
      elevationReq.ElevationRequirementVariantTypeSubtypes().AltitudeASFRequirementVariantVariant(UMAA::Common::Measurement::AltitudeASFRequirementVariantType());
      elevationReq.ElevationRequirementVariantTypeSubtypes().AltitudeASFRequirementVariantVariant().altitude().altitude(elevationValue);
      break;
    case UMAA::Common::Measurement::ElevationRequirementVariantTypeEnum::ALTITUDEGEODETICREQUIREMENTVARIANT_D:
      elevationReq.ElevationRequirementVariantTypeSubtypes().AltitudeGeodeticRequirementVariantVariant(UMAA::Common::Measurement::AltitudeGeodeticRequirementVariantType());
      elevationReq.ElevationRequirementVariantTypeSubtypes().AltitudeGeodeticRequirementVariantVariant().altitude().altitude(elevationValue);
      break;
    case UMAA::Common::Measurement::ElevationRequirementVariantTypeEnum::ALTITUDEMSLREQUIREMENTVARIANT_D:
      elevationReq.ElevationRequirementVariantTypeSubtypes().AltitudeMSLRequirementVariantVariant(UMAA::Common::Measurement::AltitudeMSLRequirementVariantType());
      elevationReq.ElevationRequirementVariantTypeSubtypes().AltitudeMSLRequirementVariantVariant().altitude().altitude(elevationValue);
      break;
    case UMAA::Common::Measurement::ElevationRequirementVariantTypeEnum::ALTITUDERATEASFREQUIREMENTVARIANT_D:
      elevationReq.ElevationRequirementVariantTypeSubtypes().AltitudeRateASFRequirementVariantVariant(UMAA::Common::Measurement::AltitudeRateASFRequirementVariantType());
      elevationReq.ElevationRequirementVariantTypeSubtypes().AltitudeRateASFRequirementVariantVariant().altitudeRate().altitudeRate(elevationValue);
      break;
    case UMAA::Common::Measurement::ElevationRequirementVariantTypeEnum::DEPTHRATEREQUIREMENTVARIANT_D:
      elevationReq.ElevationRequirementVariantTypeSubtypes().DepthRateRequirementVariantVariant(UMAA::Common::Measurement::DepthRateRequirementVariantType());
      elevationReq.ElevationRequirementVariantTypeSubtypes().DepthRateRequirementVariantVariant().depthRate().depthRate(elevationValue);
      break;
    case UMAA::Common::Measurement::ElevationRequirementVariantTypeEnum::DEPTHREQUIREMENTVARIANT_D:
      elevationReq.ElevationRequirementVariantTypeSubtypes().DepthRequirementVariantVariant(UMAA::Common::Measurement::DepthRequirementVariantType());
      elevationReq.ElevationRequirementVariantTypeSubtypes().DepthRequirementVariantVariant().depth().depth(elevationValue);
      break;
  }

  vector.elevation(elevationReq);

  return vector;
}

GlobalVectorCommandType generateDirectionVector(
    UMAA::Common::Orientation::DirectionRequirementVariantTypeEnum directionEnum, flt64_t directionValue ) {
  GlobalVectorCommandType vector = getNominalGvCommand();

  UMAA::Common::Orientation::DirectionRequirementVariantType directionReq;

  switch (directionEnum) {
    case UMAA::Common::Orientation::DirectionRequirementVariantTypeEnum::DIRECTIONCURRENTREQUIREMENTVARIANT_D:
      directionReq.DirectionRequirementVariantTypeSubtypes().DirectionCurrentRequirementVariantVariant(UMAA::Common::Orientation::DirectionCurrentRequirementVariantType());
      directionReq.DirectionRequirementVariantTypeSubtypes().DirectionCurrentRequirementVariantVariant().direction().direction(directionValue);
      break;
    case UMAA::Common::Orientation::DirectionRequirementVariantTypeEnum::DIRECTIONMAGNETICNORTHREQUIREMENTVARIANT_D:
      directionReq.DirectionRequirementVariantTypeSubtypes().DirectionMagneticNorthRequirementVariantVariant(UMAA::Common::Orientation::DirectionMagneticNorthRequirementVariantType());
      directionReq.DirectionRequirementVariantTypeSubtypes().DirectionMagneticNorthRequirementVariantVariant().direction().direction(directionValue);
      break;
    case UMAA::Common::Orientation::DirectionRequirementVariantTypeEnum::DIRECTIONTRUENORTHREQUIREMENTVARIANT_D:
      directionReq.DirectionRequirementVariantTypeSubtypes().DirectionTrueNorthRequirementVariantVariant(UMAA::Common::Orientation::DirectionTrueNorthRequirementVariantType());
      directionReq.DirectionRequirementVariantTypeSubtypes().DirectionTrueNorthRequirementVariantVariant().direction().direction(directionValue);
      break;
    case UMAA::Common::Orientation::DirectionRequirementVariantTypeEnum::DIRECTIONTURNRATEREQUIREMENTVARIANT_D:
      directionReq.DirectionRequirementVariantTypeSubtypes().DirectionTurnRateRequirementVariantVariant(UMAA::Common::Orientation::DirectionTurnRateRequirementVariantType());
      directionReq.DirectionRequirementVariantTypeSubtypes().DirectionTurnRateRequirementVariantVariant().directionRate().directionRate(directionValue);
      break;
    case UMAA::Common::Orientation::DirectionRequirementVariantTypeEnum::DIRECTIONWINDREQUIREMENTVARIANT_D:
      directionReq.DirectionRequirementVariantTypeSubtypes().DirectionWindRequirementVariantVariant(UMAA::Common::Orientation::DirectionWindRequirementVariantType());
      directionReq.DirectionRequirementVariantTypeSubtypes().DirectionWindRequirementVariantVariant().direction().direction(directionValue);
      break;
  }

  vector.direction(directionReq);

  return vector;
}


TEST(VectorUtilsTest, testAngleDiffRadians) {
  flt64_t angleA = 0;
  flt64_t angleB = M_PI;
  EXPECT_DOUBLE_EQ(arl::algorithm::angleDiffRadians(angleA, angleB), M_PI);

  angleA = -M_PI_2;
  angleB = M_PI_2;
  EXPECT_DOUBLE_EQ(arl::algorithm::angleDiffRadians(angleA, angleB), M_PI);

  angleA = -M_PI_2;
  angleB = M_PI;
  EXPECT_DOUBLE_EQ(arl::algorithm::angleDiffRadians(angleA, angleB), M_PI_2);

  angleA = 0;
  angleB = 2 * M_PI;
  EXPECT_DOUBLE_EQ(arl::algorithm::angleDiffRadians(angleA, angleB), 0);

  angleA = 0;
  angleB = 2 * M_PI - M_PI_4;
  EXPECT_DOUBLE_EQ(arl::algorithm::angleDiffRadians(angleA, angleB), -M_PI_4);
}

//! \brief Test verifies the guard clause will return false if commanded directions are different.
TEST(VectorUtilsTest, areVectorCommandsSimilar_differentDirectionVariants) {
  GlobalVectorCommandType currentCmd;
  currentCmd.directionMode(UMAA::Common::MaritimeEnumeration::DirectionModeEnumModule::DirectionModeEnumType::COURSE);

  GlobalVectorCommandType newCmd;
  newCmd.directionMode(UMAA::Common::MaritimeEnumeration::DirectionModeEnumModule::DirectionModeEnumType::HEADING);
  EXPECT_FALSE(arl::algorithm::areVectorCommandsSimilar(currentCmd, newCmd, 0, 0, 0, 0));

  currentCmd.direction().DirectionRequirementVariantTypeSubtypes().DirectionTrueNorthRequirementVariantVariant(UMAA::Common::Orientation::DirectionTrueNorthRequirementVariantType());
  currentCmd.direction().DirectionRequirementVariantTypeSubtypes().DirectionTrueNorthRequirementVariantVariant().
    direction().direction(0);

  newCmd.direction().DirectionRequirementVariantTypeSubtypes().DirectionMagneticNorthRequirementVariantVariant(UMAA::Common::Orientation::DirectionMagneticNorthRequirementVariantType());
  newCmd.direction().DirectionRequirementVariantTypeSubtypes().DirectionMagneticNorthRequirementVariantVariant().
    direction().direction(0);
  newCmd.directionMode(UMAA::Common::MaritimeEnumeration::DirectionModeEnumModule::DirectionModeEnumType::COURSE);

  EXPECT_FALSE(arl::algorithm::areVectorCommandsSimilar(currentCmd, newCmd, 0, 0, 0, 0));
}

//! \brief Test verifies the guard clause will return false if
TEST(VectorUtilsTest, areVectorCommandsSimilar_differentSpeedVariants) {
  auto directionEnum = UMAA::Common::MaritimeEnumeration::DirectionModeEnumModule::DirectionModeEnumType::COURSE;
  flt64_t SPEED_CONSTANT = 10;

  // Populate the "current command" to compare against to test the guard clauses.
  GlobalVectorCommandType currentCmd;
  currentCmd.directionMode(directionEnum);

  // Set speed of one type.
  currentCmd.speed().SpeedRequirementVariantTypeSubtypes().GroundSpeedRequirementVariantVariant(UMAA::Common::Speed::GroundSpeedRequirementVariantType());
  currentCmd.speed().SpeedRequirementVariantTypeSubtypes().GroundSpeedRequirementVariantVariant().speed().speed(SPEED_CONSTANT);

  // Make a copy of the current command and differ the speed.
  GlobalVectorCommandType newCmd = currentCmd;
  newCmd.speed().SpeedRequirementVariantTypeSubtypes().AirSpeedRequirementVariantVariant(UMAA::Common::Speed::AirSpeedRequirementVariantType());
  newCmd.speed().SpeedRequirementVariantTypeSubtypes().AirSpeedRequirementVariantVariant().speed().speed(SPEED_CONSTANT);

  EXPECT_FALSE(arl::algorithm::areVectorCommandsSimilar(currentCmd, newCmd, 0, 0, 0, 0));
}

TEST(VectorUtilsTest, areVectorsSimilarPitchChangeTest) {
  UMAA::Common::Orientation::PitchYNEDRequirement pitch;
  pitch.pitch().pitch(0);
  GlobalVectorCommandType currentCmd;
  currentCmd.depthChangePitch(pitch);
  GlobalVectorCommandType newCmd;
  pitch.pitch().pitch(0.5);
  newCmd.depthChangePitch(pitch);

  EXPECT_FALSE(arl::algorithm::areVectorCommandsSimilar(currentCmd, newCmd, 0, 0, 0, 0.1));

  newCmd.depthChangePitch().reset();
  EXPECT_FALSE(arl::algorithm::areVectorCommandsSimilar(currentCmd, newCmd, 0, 0, 0, 0.1));
}

TEST(VectorUtilsTest, areVectorsSimilarDirectionTest) {
  std::set<UMAA::Common::Orientation::DirectionRequirementVariantTypeEnum> enumSet = {
    UMAA::Common::Orientation::DirectionRequirementVariantTypeEnum::DIRECTIONCURRENTREQUIREMENTVARIANT_D,
    UMAA::Common::Orientation::DirectionRequirementVariantTypeEnum::DIRECTIONMAGNETICNORTHREQUIREMENTVARIANT_D,
    UMAA::Common::Orientation::DirectionRequirementVariantTypeEnum::DIRECTIONTRUENORTHREQUIREMENTVARIANT_D,
    UMAA::Common::Orientation::DirectionRequirementVariantTypeEnum::DIRECTIONTURNRATEREQUIREMENTVARIANT_D,
    UMAA::Common::Orientation::DirectionRequirementVariantTypeEnum::DIRECTIONWINDREQUIREMENTVARIANT_D};

  for (auto itr = enumSet.begin(); itr != enumSet.end(); itr++) {
    GlobalVectorCommandType vectorAA = generateDirectionVector(*itr, 1.00);
    GlobalVectorCommandType vectorAB = generateDirectionVector(*itr, 0.50);
    EXPECT_FALSE(arl::algorithm::areVectorCommandsSimilar(vectorAA, vectorAB, 0.1, 0.1, 0.1));
  }
}

TEST(VectorUtilsTest, areVectorsSimilarElevationTest) {
  std::set<UMAA::Common::Measurement::ElevationRequirementVariantTypeEnum> enumSet = {
    UMAA::Common::Measurement::ElevationRequirementVariantTypeEnum::ALTITUDEAGLREQUIREMENTVARIANT_D,
    UMAA::Common::Measurement::ElevationRequirementVariantTypeEnum::ALTITUDEASFREQUIREMENTVARIANT_D,
    UMAA::Common::Measurement::ElevationRequirementVariantTypeEnum::ALTITUDEGEODETICREQUIREMENTVARIANT_D,
    UMAA::Common::Measurement::ElevationRequirementVariantTypeEnum::ALTITUDEMSLREQUIREMENTVARIANT_D,
    UMAA::Common::Measurement::ElevationRequirementVariantTypeEnum::ALTITUDERATEASFREQUIREMENTVARIANT_D,
    UMAA::Common::Measurement::ElevationRequirementVariantTypeEnum::DEPTHRATEREQUIREMENTVARIANT_D,
    UMAA::Common::Measurement::ElevationRequirementVariantTypeEnum::DEPTHREQUIREMENTVARIANT_D};

  std::set<UMAA::Common::Measurement::ElevationRequirementVariantTypeEnum>::iterator itr;
  for (itr = enumSet.begin(); itr != enumSet.end(); itr++) {
    GlobalVectorCommandType vectorAA = generateElevationVector(*itr, 1.00);
    GlobalVectorCommandType vectorAB = generateElevationVector(*itr, 0.50);
    EXPECT_FALSE(arl::algorithm::areVectorCommandsSimilar(vectorAA, vectorAB, 0.1, 0.1, 0.1));
  }

  GlobalVectorCommandType vectorA = generateElevationVector(
    UMAA::Common::Measurement::ElevationRequirementVariantTypeEnum::ALTITUDEMSLREQUIREMENTVARIANT_D, 1.00);
  GlobalVectorCommandType vectorB = generateElevationVector(
    UMAA::Common::Measurement::ElevationRequirementVariantTypeEnum::ALTITUDEASFREQUIREMENTVARIANT_D, 1.00);
  EXPECT_FALSE(arl::algorithm::areVectorCommandsSimilar(vectorA, vectorB, 0.1, 0.1, 0.1));

  vectorA.elevation().reset();
  EXPECT_FALSE(arl::algorithm::areVectorCommandsSimilar(vectorA, vectorB, 0.1, 0.1, 0.1));
}

//! \brief Test Verifying logic thresholds are correct for the same speed values
TEST(VectorUtilsTest, areVectorCommandsSimilar_similarSpeedCalculationCheck) {
  GlobalVectorCommandType currentCmd = getNominalGvCommand();
  GlobalVectorCommandType newCmd = getNominalGvCommand();

  // Verify current Command matches current Command
  EXPECT_TRUE(arl::algorithm::areVectorCommandsSimilar(currentCmd, newCmd, 0, 0, 0, 0));

  // Verify current command matches itself with high tolerance
  EXPECT_TRUE(arl::algorithm::areVectorCommandsSimilar(currentCmd, newCmd, 0, 1, 0, 0));
}

//! \brief Test verifies the guard clause will return false if
TEST(VectorUtilsTest, areVectorCommandsSimilar_engineRpmVariantCalculation) {
  const uint32_t SPEED_CONSTANT = 10;

  // Populate the "current command" to compare against to test the guard clauses.
  GlobalVectorCommandType currentCmd = getNominalGvCommand();
  GlobalVectorCommandType newCmd = getNominalGvCommand();

  // Set speed of one type.
  currentCmd.speed().SpeedRequirementVariantTypeSubtypes().EngineRPMSpeedRequirementVariantVariant(UMAA::Common::Speed::EngineRPMSpeedRequirementVariantType());
  currentCmd.speed().SpeedRequirementVariantTypeSubtypes().EngineRPMSpeedRequirementVariantVariant().rpm().speed(SPEED_CONSTANT);

  newCmd.speed(currentCmd.speed());

  const uint32_t SMALL_TOLERANCE = 1;
  const uint32_t HIGH_TOLERANCE = 2;
  EXPECT_TRUE(arl::algorithm::areVectorCommandsSimilar(currentCmd, newCmd, 0, SMALL_TOLERANCE, 0, 0));

  // Slightly adjust rpm speed down less than the current tolerance
  uint32_t newSpeed = SPEED_CONSTANT - HIGH_TOLERANCE;
  newCmd.speed().SpeedRequirementVariantTypeSubtypes().EngineRPMSpeedRequirementVariantVariant().rpm().speed(newSpeed);

  // such a small change with a high tolerance means the differenc will be the same
  EXPECT_TRUE(arl::algorithm::areVectorCommandsSimilar(currentCmd, newCmd, 0, HIGH_TOLERANCE, 0, 0));

  // Adjust the tolerance to be close to the perceived change.
  EXPECT_FALSE(arl::algorithm::areVectorCommandsSimilar(currentCmd, newCmd, 0, SMALL_TOLERANCE, 0, 0));

  // Slightly adjust rpm speed up more than current tolerance val
  newSpeed = SPEED_CONSTANT + HIGH_TOLERANCE;
  newCmd.speed().SpeedRequirementVariantTypeSubtypes().EngineRPMSpeedRequirementVariantVariant().rpm().speed(newSpeed);

  // Adjust the tolerance to be close to the perceived change.
  EXPECT_FALSE(arl::algorithm::areVectorCommandsSimilar(currentCmd, newCmd, 0, SMALL_TOLERANCE, 0, 0));
}

TEST(VectorUtilsTest, areVectorCommandsSimilar_airSpeedVariantCalculation) {
  const flt64_t SPEED_CONSTANT = 10;

  // Populate the "current command" to compare against to test the guard clauses.
  GlobalVectorCommandType currentCmd = getNominalGvCommand();
  GlobalVectorCommandType newCmd = getNominalGvCommand();

  // Set speed of one type.
  currentCmd.speed().SpeedRequirementVariantTypeSubtypes().AirSpeedRequirementVariantVariant(UMAA::Common::Speed::AirSpeedRequirementVariantType());
  currentCmd.speed().SpeedRequirementVariantTypeSubtypes().AirSpeedRequirementVariantVariant().speed().speed(SPEED_CONSTANT);

  newCmd.speed(currentCmd.speed());

  const flt64_t SMALL_TOLERANCE = 0.1;
  const flt64_t HIGH_TOLERANCE = 1;
  EXPECT_TRUE(arl::algorithm::areVectorCommandsSimilar(currentCmd, newCmd, 0, SMALL_TOLERANCE, 0, 0));

  // Slightly adjust rpm speed down less than the current tolerance
  flt64_t newSpeed = SPEED_CONSTANT - SMALL_TOLERANCE - (SMALL_TOLERANCE/100);
  newCmd.speed().SpeedRequirementVariantTypeSubtypes().AirSpeedRequirementVariantVariant().speed().speed(newSpeed);

  // such a small change with a high tolerance means the differenc will be the same
  EXPECT_TRUE(arl::algorithm::areVectorCommandsSimilar(currentCmd, newCmd, 0, HIGH_TOLERANCE, 0, 0));

  // Adjust the tolerance to be close to the perceived change.
  EXPECT_FALSE(arl::algorithm::areVectorCommandsSimilar(currentCmd, newCmd, 0, SMALL_TOLERANCE, 0, 0));

  // Slightly adjust rpm speed up more than current tolerance val
  newSpeed = SPEED_CONSTANT + SMALL_TOLERANCE + (SMALL_TOLERANCE/100);
  newCmd.speed().SpeedRequirementVariantTypeSubtypes().AirSpeedRequirementVariantVariant().speed().speed(newSpeed);

  // Adjust the tolerance to be close to the perceived change.
  EXPECT_FALSE(arl::algorithm::areVectorCommandsSimilar(currentCmd, newCmd, 0, SMALL_TOLERANCE, 0, 0));
}

TEST(VectorUtilsTest, areVectorCommandsSimilar_waterSpeedVariantCalculation) {
  const flt64_t SPEED_CONSTANT = 10;

  // Populate the "current command" to compare against to test the guard clauses.
  GlobalVectorCommandType currentCmd = getNominalGvCommand();
  GlobalVectorCommandType newCmd = getNominalGvCommand();

  // Set speed of one type.
  currentCmd.speed().SpeedRequirementVariantTypeSubtypes().WaterSpeedRequirementVariantVariant(UMAA::Common::Speed::WaterSpeedRequirementVariantType());
  currentCmd.speed().SpeedRequirementVariantTypeSubtypes().WaterSpeedRequirementVariantVariant().speed().speed(SPEED_CONSTANT);

  newCmd.speed(currentCmd.speed());

  const flt64_t SMALL_TOLERANCE = 0.1;
  const flt64_t HIGH_TOLERANCE = 1;
  EXPECT_TRUE(arl::algorithm::areVectorCommandsSimilar(currentCmd, newCmd, 0, SMALL_TOLERANCE, 0, 0));

  // Slightly adjust rpm speed down less than the current tolerance
  flt64_t newSpeed = SPEED_CONSTANT - SMALL_TOLERANCE - (SMALL_TOLERANCE/100);
  newCmd.speed().SpeedRequirementVariantTypeSubtypes().WaterSpeedRequirementVariantVariant().speed().speed(newSpeed);

  // such a small change with a high tolerance means the differenc will be the same
  EXPECT_TRUE(arl::algorithm::areVectorCommandsSimilar(currentCmd, newCmd, 0, HIGH_TOLERANCE, 0, 0));

  // Adjust the tolerance to be close to the perceived change.
  EXPECT_FALSE(arl::algorithm::areVectorCommandsSimilar(currentCmd, newCmd, 0, SMALL_TOLERANCE, 0, 0));

  // Slightly adjust rpm speed up more than current tolerance val
  newSpeed = SPEED_CONSTANT + SMALL_TOLERANCE + (SMALL_TOLERANCE/100);
  newCmd.speed().SpeedRequirementVariantTypeSubtypes().WaterSpeedRequirementVariantVariant().speed().speed(newSpeed);

  // Adjust the tolerance to be close to the perceived change.
  EXPECT_FALSE(arl::algorithm::areVectorCommandsSimilar(currentCmd, newCmd, 0, SMALL_TOLERANCE, 0, 0));
}

//! \brief Test verifying logic will return false if the speed types differ
TEST(VectorUtilsTest, areVectorCommandsSimilar_differentVehicleSpeedMode) {

  // Populate the "current command" to compare against to test the guard clauses.
  GlobalVectorCommandType currentCmd = getNominalGvCommand();
  currentCmd.speed().SpeedRequirementVariantTypeSubtypes().VehicleSpeedModeRequirementVariantVariant(UMAA::Common::Speed::VehicleSpeedModeRequirementVariantType());

  GlobalVectorCommandType newCmd = getNominalGvCommand();
  newCmd.speed().SpeedRequirementVariantTypeSubtypes().VehicleSpeedModeRequirementVariantVariant(UMAA::Common::Speed::VehicleSpeedModeRequirementVariantType());

  auto currentSpeedMode = UMAA::Common::MaritimeEnumeration::VehicleSpeedModeEnumModule::VehicleSpeedModeEnumType::LRC;
  currentCmd.speed().SpeedRequirementVariantTypeSubtypes().VehicleSpeedModeRequirementVariantVariant().mode(currentSpeedMode);

  // Set the new command to a different speed mode calc.
  auto diffSpeedMode = UMAA::Common::MaritimeEnumeration::VehicleSpeedModeEnumModule::VehicleSpeedModeEnumType::MEC;
  newCmd.speed().SpeedRequirementVariantTypeSubtypes().VehicleSpeedModeRequirementVariantVariant().mode(diffSpeedMode);

  const flt64_t SMALL_TOLERANCE = 0.1;
  const flt64_t HIGH_TOLERANCE = 1;

  // Both tolerances should result in bad commands
  EXPECT_FALSE(arl::algorithm::areVectorCommandsSimilar(currentCmd, newCmd, 0, SMALL_TOLERANCE, 0, 0));
  EXPECT_FALSE(arl::algorithm::areVectorCommandsSimilar(currentCmd, newCmd, 0, HIGH_TOLERANCE, 0, 0));
}

TEST(VectorUtilsTest, areVectorCommandsSimilar_sameVehicleSpeedMode) {
  // Populate the "current command" to compare against to test the guard clauses.
  GlobalVectorCommandType currentCmd = getNominalGvCommand();
  currentCmd.speed().SpeedRequirementVariantTypeSubtypes().VehicleSpeedModeRequirementVariantVariant(UMAA::Common::Speed::VehicleSpeedModeRequirementVariantType());

  GlobalVectorCommandType newCmd = getNominalGvCommand();
  newCmd.speed().SpeedRequirementVariantTypeSubtypes().VehicleSpeedModeRequirementVariantVariant(UMAA::Common::Speed::VehicleSpeedModeRequirementVariantType());

  auto currentSpeedMode = UMAA::Common::MaritimeEnumeration::VehicleSpeedModeEnumModule::VehicleSpeedModeEnumType::LRC;
  currentCmd.speed().SpeedRequirementVariantTypeSubtypes().VehicleSpeedModeRequirementVariantVariant().mode(currentSpeedMode);

  // Set the new command to the same speed mode calc.
  newCmd.speed().SpeedRequirementVariantTypeSubtypes().VehicleSpeedModeRequirementVariantVariant().mode(currentSpeedMode);

  const flt64_t SMALL_TOLERANCE = 0.1;
  const flt64_t HIGH_TOLERANCE = 1;

  // Both tolerances should result in the commands being similar
  EXPECT_TRUE(arl::algorithm::areVectorCommandsSimilar(currentCmd, newCmd, 0, SMALL_TOLERANCE, 0, 0));
  EXPECT_TRUE(arl::algorithm::areVectorCommandsSimilar(currentCmd, newCmd, 0, HIGH_TOLERANCE, 0, 0));
}

