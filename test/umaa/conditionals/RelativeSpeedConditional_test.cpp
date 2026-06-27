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

#include "RelativeSpeedConditional.h"
#include "UuidFactory.h"

using UMAA::Common::MaritimeEnumeration::ConditionalOperatorEnumModule::ConditionalOperatorEnumType;

class RelativeSpeedConditionalTest : public ::testing::Test {
 protected:
  void SetUp() override {
    baseConditional_ = arlcore::umaa::conditional::ConditionalType(conditionalId, conditionalName, specializationId, specializationTimestamp, arlcore::umaa::conditional::RelativeSpeedConditionalTypeTopic);
    specializedConditional_ = arlcore::umaa::conditional::RelativeSpeedConditionalType(defaultOperator, defaultRelativeSpeed, specializationTimestamp, specializationId);
  }

  void TearDown() override {
    
  }

  arlcore::umaa::conditional::ConditionalType baseConditional_;
  arlcore::umaa::conditional::RelativeSpeedConditionalType specializedConditional_;

  static const arlcore::NumericGuid conditionalId;
  static const arlcore::NumericGuid specializationId;
  static const std::string conditionalName;
  static const UMAA::Common::Measurement::DateTime specializationTimestamp;
  static const flt64_t defaultRelativeSpeed;
  static const ConditionalOperatorEnumType defaultOperator;
};

const arlcore::NumericGuid RelativeSpeedConditionalTest::conditionalId = arlcore::UuidFactory::getInstance().parseGuidFromString("00000000-0000-0000-0000-100000000000");
const arlcore::NumericGuid RelativeSpeedConditionalTest::specializationId = arlcore::UuidFactory::getInstance().parseGuidFromString("00000000-0000-0000-0000-200000000000");
const std::string RelativeSpeedConditionalTest::conditionalName = "TestRelativeSpeedConditional";
const UMAA::Common::Measurement::DateTime RelativeSpeedConditionalTest::specializationTimestamp = UMAA::Common::Measurement::DateTime(1, 100);
const flt64_t RelativeSpeedConditionalTest::defaultRelativeSpeed = 12.34;
const ConditionalOperatorEnumType RelativeSpeedConditionalTest::defaultOperator = ConditionalOperatorEnumType::GREATER_THAN;

TEST_F(RelativeSpeedConditionalTest, testCustomGetters) {
  arlcore::umaa::conditional::RelativeSpeedConditional relativeSpeed(baseConditional_, specializedConditional_);
  EXPECT_EQ(relativeSpeed.getSpecializedConditional(), specializedConditional_);
  EXPECT_EQ(relativeSpeed.getConditionalSpeed(), defaultRelativeSpeed);
  EXPECT_EQ(relativeSpeed.getConditionalOperatorEnum(), defaultOperator);
  auto deps = relativeSpeed.getDependencies();
  EXPECT_EQ(deps.first, std::nullopt);
  EXPECT_EQ(deps.second, std::nullopt);
}

TEST_F(RelativeSpeedConditionalTest, noSpeedData) {
  arlcore::umaa::conditional::RelativeSpeedConditional relativeSpeed(baseConditional_, specializedConditional_);
  EXPECT_EQ(relativeSpeed.evaluateConditional(), std::nullopt);
}

TEST_F(RelativeSpeedConditionalTest, testGreaterThan) {
  arlcore::umaa::conditional::RelativeSpeedConditional relativeSpeed(baseConditional_, specializedConditional_);
  arlcore::umaa::conditional::SpeedReportType ssData;

  ssData.speedThroughWater(12.45);
  relativeSpeed.update(ssData);
  ASSERT_TRUE(relativeSpeed.evaluateConditional().has_value());
  EXPECT_TRUE(relativeSpeed.evaluateConditional().value());
  ssData.speedThroughWater(defaultRelativeSpeed);
  relativeSpeed.update(ssData);
  ASSERT_TRUE(relativeSpeed.evaluateConditional().has_value());
  EXPECT_FALSE(relativeSpeed.evaluateConditional().value());
  ssData.speedThroughWater(2.12);
  relativeSpeed.update(ssData);
  ASSERT_TRUE(relativeSpeed.evaluateConditional().has_value());
  EXPECT_FALSE(relativeSpeed.evaluateConditional().value());
}

TEST_F(RelativeSpeedConditionalTest, testGreaterThanOrEqualTo) {
  specializedConditional_.conditionalOp(ConditionalOperatorEnumType::GREATER_THAN_OR_EQUAL_TO);
  arlcore::umaa::conditional::RelativeSpeedConditional relativeSpeed(baseConditional_, specializedConditional_);
  arlcore::umaa::conditional::SpeedReportType ssData;

  ssData.speedThroughWater(12.45);
  relativeSpeed.update(ssData);
  ASSERT_TRUE(relativeSpeed.evaluateConditional().has_value());
  EXPECT_TRUE(relativeSpeed.evaluateConditional().value());
  ssData.speedThroughWater(defaultRelativeSpeed);
  relativeSpeed.update(ssData);
  ASSERT_TRUE(relativeSpeed.evaluateConditional().has_value());
  EXPECT_TRUE(relativeSpeed.evaluateConditional().value());
  ssData.speedThroughWater(2.12);
  relativeSpeed.update(ssData);
  ASSERT_TRUE(relativeSpeed.evaluateConditional().has_value());
  EXPECT_FALSE(relativeSpeed.evaluateConditional().value());
}

TEST_F(RelativeSpeedConditionalTest, testLessThan) {
  specializedConditional_.conditionalOp(ConditionalOperatorEnumType::LESS_THAN);
  arlcore::umaa::conditional::RelativeSpeedConditional relativeSpeed(baseConditional_, specializedConditional_);
  arlcore::umaa::conditional::SpeedReportType ssData;

  ssData.speedThroughWater(12.45);
  relativeSpeed.update(ssData);
  ASSERT_TRUE(relativeSpeed.evaluateConditional().has_value());
  EXPECT_FALSE(relativeSpeed.evaluateConditional().value());
  ssData.speedThroughWater(defaultRelativeSpeed);
  relativeSpeed.update(ssData);
  ASSERT_TRUE(relativeSpeed.evaluateConditional().has_value());
  EXPECT_FALSE(relativeSpeed.evaluateConditional().value());
  ssData.speedThroughWater(2.12);
  relativeSpeed.update(ssData);
  ASSERT_TRUE(relativeSpeed.evaluateConditional().has_value());
  EXPECT_TRUE(relativeSpeed.evaluateConditional().value());
}

TEST_F(RelativeSpeedConditionalTest, testLessThanOrEqualTo) {
  specializedConditional_.conditionalOp(ConditionalOperatorEnumType::LESS_THAN_OR_EQUAL_TO);
  arlcore::umaa::conditional::RelativeSpeedConditional relativeSpeed(baseConditional_, specializedConditional_);
  arlcore::umaa::conditional::SpeedReportType ssData;

  ssData.speedThroughWater(12.45);
  relativeSpeed.update(ssData);
  ASSERT_TRUE(relativeSpeed.evaluateConditional().has_value());
  EXPECT_FALSE(relativeSpeed.evaluateConditional().value());
  ssData.speedThroughWater(defaultRelativeSpeed);
  relativeSpeed.update(ssData);
  ASSERT_TRUE(relativeSpeed.evaluateConditional().has_value());
  EXPECT_TRUE(relativeSpeed.evaluateConditional().value());
  ssData.speedThroughWater(2.12);
  relativeSpeed.update(ssData);
  ASSERT_TRUE(relativeSpeed.evaluateConditional().has_value());
  EXPECT_TRUE(relativeSpeed.evaluateConditional().value());
}