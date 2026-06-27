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

#include "PitchRateConditional.h"
#include "UuidFactory.h"

using UMAA::Common::MaritimeEnumeration::ConditionalOperatorEnumModule::ConditionalOperatorEnumType;

class PitchRateConditionalTest : public ::testing::Test {
 protected:
  void SetUp() override {
    baseConditional_ = arlcore::umaa::conditional::ConditionalType(conditionalId, conditionalName, specializationId, specializationTimestamp, arlcore::umaa::conditional::PitchRateConditionalTypeTopic);
    specializedConditional_ = arlcore::umaa::conditional::PitchRateConditionalType(defaultOperator, defaultPitchRate, specializationTimestamp, specializationId);
  }

  void TearDown() override {
    
  }

  arlcore::umaa::conditional::ConditionalType baseConditional_;
  arlcore::umaa::conditional::PitchRateConditionalType specializedConditional_;

  static const arlcore::NumericGuid conditionalId;
  static const arlcore::NumericGuid specializationId;
  static const std::string conditionalName;
  static const UMAA::Common::Measurement::DateTime specializationTimestamp;
  static const flt64_t defaultPitchRate;
  static const ConditionalOperatorEnumType defaultOperator;
};

const arlcore::NumericGuid PitchRateConditionalTest::conditionalId = arlcore::UuidFactory::getInstance().parseGuidFromString("00000000-0000-0000-0000-100000000000");
const arlcore::NumericGuid PitchRateConditionalTest::specializationId = arlcore::UuidFactory::getInstance().parseGuidFromString("00000000-0000-0000-0000-200000000000");
const std::string PitchRateConditionalTest::conditionalName = "TestPitchRateConditional";
const UMAA::Common::Measurement::DateTime PitchRateConditionalTest::specializationTimestamp = UMAA::Common::Measurement::DateTime(1, 100);
const flt64_t PitchRateConditionalTest::defaultPitchRate = 12.34;
const ConditionalOperatorEnumType PitchRateConditionalTest::defaultOperator = ConditionalOperatorEnumType::GREATER_THAN;

TEST_F(PitchRateConditionalTest, testCustomGetters) {
  arlcore::umaa::conditional::PitchRateConditional pitchRate(baseConditional_, specializedConditional_);
  EXPECT_EQ(pitchRate.getSpecializedConditional(), specializedConditional_);
  EXPECT_EQ(pitchRate.getConditionalPitchRate(), defaultPitchRate);
  EXPECT_EQ(pitchRate.getConditionalOperatorEnum(), defaultOperator);
  auto deps = pitchRate.getDependencies();
  EXPECT_EQ(deps.first, std::nullopt);
  EXPECT_EQ(deps.second, std::nullopt);
}

TEST_F(PitchRateConditionalTest, noVelocityData) {
  arlcore::umaa::conditional::PitchRateConditional pitchRate(baseConditional_, specializedConditional_);
  EXPECT_EQ(pitchRate.evaluateConditional(), std::nullopt);
}

TEST_F(PitchRateConditionalTest, testGreaterThan) {
  arlcore::umaa::conditional::PitchRateConditional pitchRate(baseConditional_, specializedConditional_);
  arlcore::umaa::conditional::VelocityReportType vData;

  vData.attitudeRate().pitchRate(12.45);
  pitchRate.update(vData);
  ASSERT_TRUE(pitchRate.evaluateConditional().has_value());
  EXPECT_TRUE(pitchRate.evaluateConditional().value());
  vData.attitudeRate().pitchRate(defaultPitchRate);
  pitchRate.update(vData);
  ASSERT_TRUE(pitchRate.evaluateConditional().has_value());
  EXPECT_FALSE(pitchRate.evaluateConditional().value());
  vData.attitudeRate().pitchRate(2.12);
  pitchRate.update(vData);
  ASSERT_TRUE(pitchRate.evaluateConditional().has_value());
  EXPECT_FALSE(pitchRate.evaluateConditional().value());
}

TEST_F(PitchRateConditionalTest, testGreaterThanOrEqualTo) {
  specializedConditional_.conditionalOp(ConditionalOperatorEnumType::GREATER_THAN_OR_EQUAL_TO);
  arlcore::umaa::conditional::PitchRateConditional pitchRate(baseConditional_, specializedConditional_);
  arlcore::umaa::conditional::VelocityReportType vData;

  vData.attitudeRate().pitchRate(12.45);
  pitchRate.update(vData);
  ASSERT_TRUE(pitchRate.evaluateConditional().has_value());
  EXPECT_TRUE(pitchRate.evaluateConditional().value());
  vData.attitudeRate().pitchRate(defaultPitchRate);
  pitchRate.update(vData);
  ASSERT_TRUE(pitchRate.evaluateConditional().has_value());
  EXPECT_TRUE(pitchRate.evaluateConditional().value());
  vData.attitudeRate().pitchRate(2.12);
  pitchRate.update(vData);
  ASSERT_TRUE(pitchRate.evaluateConditional().has_value());
  EXPECT_FALSE(pitchRate.evaluateConditional().value());
}

TEST_F(PitchRateConditionalTest, testLessThan) {
  specializedConditional_.conditionalOp(ConditionalOperatorEnumType::LESS_THAN);
  arlcore::umaa::conditional::PitchRateConditional pitchRate(baseConditional_, specializedConditional_);
  arlcore::umaa::conditional::VelocityReportType vData;

  vData.attitudeRate().pitchRate(12.45);
  pitchRate.update(vData);
  ASSERT_TRUE(pitchRate.evaluateConditional().has_value());
  EXPECT_FALSE(pitchRate.evaluateConditional().value());
  vData.attitudeRate().pitchRate(defaultPitchRate);
  pitchRate.update(vData);
  ASSERT_TRUE(pitchRate.evaluateConditional().has_value());
  EXPECT_FALSE(pitchRate.evaluateConditional().value());
  vData.attitudeRate().pitchRate(2.12);
  pitchRate.update(vData);
  ASSERT_TRUE(pitchRate.evaluateConditional().has_value());
  EXPECT_TRUE(pitchRate.evaluateConditional().value());
}

TEST_F(PitchRateConditionalTest, testLessThanOrEqualTo) {
  specializedConditional_.conditionalOp(ConditionalOperatorEnumType::LESS_THAN_OR_EQUAL_TO);
  arlcore::umaa::conditional::PitchRateConditional pitchRate(baseConditional_, specializedConditional_);
  arlcore::umaa::conditional::VelocityReportType vData;
  
  vData.attitudeRate().pitchRate(12.45);
  pitchRate.update(vData);
  ASSERT_TRUE(pitchRate.evaluateConditional().has_value());
  EXPECT_FALSE(pitchRate.evaluateConditional().value());
  vData.attitudeRate().pitchRate(defaultPitchRate);
  pitchRate.update(vData);
  ASSERT_TRUE(pitchRate.evaluateConditional().has_value());
  EXPECT_TRUE(pitchRate.evaluateConditional().value());
  vData.attitudeRate().pitchRate(2.12);
  pitchRate.update(vData);
  ASSERT_TRUE(pitchRate.evaluateConditional().has_value());
  EXPECT_TRUE(pitchRate.evaluateConditional().value());
}