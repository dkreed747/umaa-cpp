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

#include "RollRateConditional.h"
#include "UuidFactory.h"

using UMAA::Common::MaritimeEnumeration::ConditionalOperatorEnumModule::ConditionalOperatorEnumType;

class RollRateConditionalTest : public ::testing::Test {
 protected:
  void SetUp() override {
    baseConditional_ = arlcore::umaa::conditional::ConditionalType(conditionalId, conditionalName, specializationId, specializationTimestamp, arlcore::umaa::conditional::RollRateConditionalTypeTopic);
    specializedConditional_ = arlcore::umaa::conditional::RollRateConditionalType(defaultOperator, defaultRollRate, specializationTimestamp, specializationId);
  }

  void TearDown() override {
    
  }

  arlcore::umaa::conditional::ConditionalType baseConditional_;
  arlcore::umaa::conditional::RollRateConditionalType specializedConditional_;

  static const arlcore::NumericGuid conditionalId;
  static const arlcore::NumericGuid specializationId;
  static const std::string conditionalName;
  static const UMAA::Common::Measurement::DateTime specializationTimestamp;
  static const flt64_t defaultRollRate;
  static const ConditionalOperatorEnumType defaultOperator;
};

const arlcore::NumericGuid RollRateConditionalTest::conditionalId = arlcore::UuidFactory::getInstance().parseGuidFromString("00000000-0000-0000-0000-100000000000");
const arlcore::NumericGuid RollRateConditionalTest::specializationId = arlcore::UuidFactory::getInstance().parseGuidFromString("00000000-0000-0000-0000-200000000000");
const std::string RollRateConditionalTest::conditionalName = "TestRollRateConditional";
const UMAA::Common::Measurement::DateTime RollRateConditionalTest::specializationTimestamp = UMAA::Common::Measurement::DateTime(1, 100);
const flt64_t RollRateConditionalTest::defaultRollRate = 12.34;
const ConditionalOperatorEnumType RollRateConditionalTest::defaultOperator = ConditionalOperatorEnumType::GREATER_THAN;

TEST_F(RollRateConditionalTest, testCustomGetters) {
  arlcore::umaa::conditional::RollRateConditional rollRate(baseConditional_, specializedConditional_);
  EXPECT_EQ(rollRate.getSpecializedConditional(), specializedConditional_);
  EXPECT_EQ(rollRate.getConditionalRollRate(), defaultRollRate);
  EXPECT_EQ(rollRate.getConditionalOperatorEnum(), defaultOperator);
  auto deps = rollRate.getDependencies();
  EXPECT_EQ(deps.first, std::nullopt);
  EXPECT_EQ(deps.second, std::nullopt);
}

TEST_F(RollRateConditionalTest, noVelocityData) {
  arlcore::umaa::conditional::RollRateConditional rollRate(baseConditional_, specializedConditional_);
  EXPECT_EQ(rollRate.evaluateConditional(), std::nullopt);
}

TEST_F(RollRateConditionalTest, testGreaterThan) {
  arlcore::umaa::conditional::RollRateConditional rollRate(baseConditional_, specializedConditional_);
  arlcore::umaa::conditional::VelocityReportType vData;

  vData.attitudeRate().rollRate(12.45);
  rollRate.update(vData);
  ASSERT_TRUE(rollRate.evaluateConditional().has_value());
  EXPECT_TRUE(rollRate.evaluateConditional().value());
  vData.attitudeRate().rollRate(defaultRollRate);
  rollRate.update(vData);
  ASSERT_TRUE(rollRate.evaluateConditional().has_value());
  EXPECT_FALSE(rollRate.evaluateConditional().value());
  vData.attitudeRate().rollRate(2.12);
  rollRate.update(vData);
  ASSERT_TRUE(rollRate.evaluateConditional().has_value());
  EXPECT_FALSE(rollRate.evaluateConditional().value());
}

TEST_F(RollRateConditionalTest, testGreaterThanOrEqualTo) {
  specializedConditional_.conditionalOp(ConditionalOperatorEnumType::GREATER_THAN_OR_EQUAL_TO);
  arlcore::umaa::conditional::RollRateConditional rollRate(baseConditional_, specializedConditional_);
  arlcore::umaa::conditional::VelocityReportType vData;

  vData.attitudeRate().rollRate(12.45);
  rollRate.update(vData);
  ASSERT_TRUE(rollRate.evaluateConditional().has_value());
  EXPECT_TRUE(rollRate.evaluateConditional().value());
  vData.attitudeRate().rollRate(defaultRollRate);
  rollRate.update(vData);
  ASSERT_TRUE(rollRate.evaluateConditional().has_value());
  EXPECT_TRUE(rollRate.evaluateConditional().value());
  vData.attitudeRate().rollRate(2.12);
  rollRate.update(vData);
  ASSERT_TRUE(rollRate.evaluateConditional().has_value());
  EXPECT_FALSE(rollRate.evaluateConditional().value());
}

TEST_F(RollRateConditionalTest, testLessThan) {
  specializedConditional_.conditionalOp(ConditionalOperatorEnumType::LESS_THAN);
  arlcore::umaa::conditional::RollRateConditional rollRate(baseConditional_, specializedConditional_);
  arlcore::umaa::conditional::VelocityReportType vData;

  vData.attitudeRate().rollRate(12.45);
  rollRate.update(vData);
  ASSERT_TRUE(rollRate.evaluateConditional().has_value());
  EXPECT_FALSE(rollRate.evaluateConditional().value());
  vData.attitudeRate().rollRate(defaultRollRate);
  rollRate.update(vData);
  ASSERT_TRUE(rollRate.evaluateConditional().has_value());
  EXPECT_FALSE(rollRate.evaluateConditional().value());
  vData.attitudeRate().rollRate(2.12);
  rollRate.update(vData);
  ASSERT_TRUE(rollRate.evaluateConditional().has_value());
  EXPECT_TRUE(rollRate.evaluateConditional().value());
}

TEST_F(RollRateConditionalTest, testLessThanOrEqualTo) {
  specializedConditional_.conditionalOp(ConditionalOperatorEnumType::LESS_THAN_OR_EQUAL_TO);
  arlcore::umaa::conditional::RollRateConditional rollRate(baseConditional_, specializedConditional_);
  arlcore::umaa::conditional::VelocityReportType vData;

  vData.attitudeRate().rollRate(12.45);
  rollRate.update(vData);
  ASSERT_TRUE(rollRate.evaluateConditional().has_value());
  EXPECT_FALSE(rollRate.evaluateConditional().value());
  vData.attitudeRate().rollRate(defaultRollRate);
  rollRate.update(vData);
  ASSERT_TRUE(rollRate.evaluateConditional().has_value());
  EXPECT_TRUE(rollRate.evaluateConditional().value());
  vData.attitudeRate().rollRate(2.12);
  rollRate.update(vData);
  ASSERT_TRUE(rollRate.evaluateConditional().has_value());
  EXPECT_TRUE(rollRate.evaluateConditional().value());
}