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

#include "SpeedConditional.h"
#include "UuidFactory.h"

using UMAA::Common::MaritimeEnumeration::ConditionalOperatorEnumModule::ConditionalOperatorEnumType;

class SpeedConditionalTest : public ::testing::Test {
 protected:
  void SetUp() override {
    baseConditional_ = arlcore::umaa::conditional::ConditionalType(conditionalId, conditionalName, specializationId, specializationTimestamp, arlcore::umaa::conditional::SpeedConditionalTypeTopic);
    specializedConditional_ = arlcore::umaa::conditional::SpeedConditionalType(defaultOperator, defaultSpeed, specializationTimestamp, specializationId);
  }

  void TearDown() override {
    
  }

  arlcore::umaa::conditional::ConditionalType baseConditional_;
  arlcore::umaa::conditional::SpeedConditionalType specializedConditional_;

  static const arlcore::NumericGuid conditionalId;
  static const arlcore::NumericGuid specializationId;
  static const std::string conditionalName;
  static const UMAA::Common::Measurement::DateTime specializationTimestamp;
  static const flt64_t defaultSpeed;
  static const ConditionalOperatorEnumType defaultOperator;
};

const arlcore::NumericGuid SpeedConditionalTest::conditionalId = arlcore::UuidFactory::getInstance().parseGuidFromString("00000000-0000-0000-0000-100000000000");
const arlcore::NumericGuid SpeedConditionalTest::specializationId = arlcore::UuidFactory::getInstance().parseGuidFromString("00000000-0000-0000-0000-200000000000");
const std::string SpeedConditionalTest::conditionalName = "TestSpeedConditional";
const UMAA::Common::Measurement::DateTime SpeedConditionalTest::specializationTimestamp = UMAA::Common::Measurement::DateTime(1, 100);
const flt64_t SpeedConditionalTest::defaultSpeed = 12.34;
const ConditionalOperatorEnumType SpeedConditionalTest::defaultOperator = ConditionalOperatorEnumType::GREATER_THAN;

TEST_F(SpeedConditionalTest, testCustomGetters) {
  arlcore::umaa::conditional::SpeedConditional speed(baseConditional_, specializedConditional_);
  EXPECT_EQ(speed.getSpecializedConditional(), specializedConditional_);
  EXPECT_EQ(speed.getConditionalSpeed(), defaultSpeed);
  EXPECT_EQ(speed.getConditionalOperatorEnum(), defaultOperator);
  auto deps = speed.getDependencies();
  EXPECT_EQ(deps.first, std::nullopt);
  EXPECT_EQ(deps.second, std::nullopt);
}

TEST_F(SpeedConditionalTest, noSpeedData) {
  arlcore::umaa::conditional::SpeedConditional speed(baseConditional_, specializedConditional_);
  EXPECT_EQ(speed.evaluateConditional(), std::nullopt);
}

TEST_F(SpeedConditionalTest, testGreaterThan) {
  arlcore::umaa::conditional::SpeedConditional speed(baseConditional_, specializedConditional_);
  arlcore::umaa::conditional::SpeedReportType ssData;

  ssData.speedOverGround(12.45);
  speed.update(ssData);
  EXPECT_TRUE(speed.evaluateConditional().has_value());
  EXPECT_TRUE(speed.evaluateConditional().value());
  ssData.speedOverGround(defaultSpeed);
  speed.update(ssData);
  ASSERT_TRUE(speed.evaluateConditional().has_value());
  EXPECT_FALSE(speed.evaluateConditional().value());
  ssData.speedOverGround(2.12);
  speed.update(ssData);
  ASSERT_TRUE(speed.evaluateConditional().has_value());
  EXPECT_FALSE(speed.evaluateConditional().value());
}

TEST_F(SpeedConditionalTest, testGreaterThanOrEqualTo) {
  specializedConditional_.conditionalOp(ConditionalOperatorEnumType::GREATER_THAN_OR_EQUAL_TO);
  arlcore::umaa::conditional::SpeedConditional speed(baseConditional_, specializedConditional_);
  arlcore::umaa::conditional::SpeedReportType ssData;

  ssData.speedOverGround(12.45);
  speed.update(ssData);
  ASSERT_TRUE(speed.evaluateConditional().has_value());
  EXPECT_TRUE(speed.evaluateConditional().value());
  ssData.speedOverGround(defaultSpeed);
  speed.update(ssData);
  ASSERT_TRUE(speed.evaluateConditional().has_value());
  EXPECT_TRUE(speed.evaluateConditional().value());
  ssData.speedOverGround(2.12);
  speed.update(ssData);
  ASSERT_TRUE(speed.evaluateConditional().has_value());
  EXPECT_FALSE(speed.evaluateConditional().value());
}

TEST_F(SpeedConditionalTest, testLessThan) {
  specializedConditional_.conditionalOp(ConditionalOperatorEnumType::LESS_THAN);
  arlcore::umaa::conditional::SpeedConditional speed(baseConditional_, specializedConditional_);
  arlcore::umaa::conditional::SpeedReportType ssData;

  ssData.speedOverGround(12.45);
  speed.update(ssData);
  ASSERT_TRUE(speed.evaluateConditional().has_value());
  EXPECT_FALSE(speed.evaluateConditional().value());
  ssData.speedOverGround(defaultSpeed);
  speed.update(ssData);
  ASSERT_TRUE(speed.evaluateConditional().has_value());
  EXPECT_FALSE(speed.evaluateConditional().value());
  ssData.speedOverGround(2.12);
  speed.update(ssData);
  ASSERT_TRUE(speed.evaluateConditional().has_value());
  EXPECT_TRUE(speed.evaluateConditional().value());
}

TEST_F(SpeedConditionalTest, testLessThanOrEqualTo) {
  specializedConditional_.conditionalOp(ConditionalOperatorEnumType::LESS_THAN_OR_EQUAL_TO);
  arlcore::umaa::conditional::SpeedConditional speed(baseConditional_, specializedConditional_);
  arlcore::umaa::conditional::SpeedReportType ssData;

  ssData.speedOverGround(12.45);
  speed.update(ssData);
  ASSERT_TRUE(speed.evaluateConditional().has_value());
  EXPECT_FALSE(speed.evaluateConditional().value());
  ssData.speedOverGround(defaultSpeed);
  speed.update(ssData);
  ASSERT_TRUE(speed.evaluateConditional().has_value());
  EXPECT_TRUE(speed.evaluateConditional().value());
  ssData.speedOverGround(2.12);
  speed.update(ssData);
  ASSERT_TRUE(speed.evaluateConditional().has_value());
  EXPECT_TRUE(speed.evaluateConditional().value());
}