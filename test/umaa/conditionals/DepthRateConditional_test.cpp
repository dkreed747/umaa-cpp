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

#include "DepthRateConditional.h"
#include "UuidFactory.h"

using UMAA::Common::MaritimeEnumeration::ConditionalOperatorEnumModule::ConditionalOperatorEnumType;

class DepthRateConditionalTest : public ::testing::Test {
 protected:
  void SetUp() override {
    baseConditional_ = arlcore::umaa::conditional::ConditionalType(conditionalId, conditionalName, specializationId, specializationTimestamp, arlcore::umaa::conditional::DepthRateConditionalTypeTopic);
    specializedConditional_ = arlcore::umaa::conditional::DepthRateConditionalType(defaultOperator, defaultDepthRate, specializationTimestamp, specializationId);
  }

  void TearDown() override {
    
  }

  arlcore::umaa::conditional::ConditionalType baseConditional_;
  arlcore::umaa::conditional::DepthRateConditionalType specializedConditional_;

  static const arlcore::NumericGuid conditionalId;
  static const arlcore::NumericGuid specializationId;
  static const std::string conditionalName;
  static const UMAA::Common::Measurement::DateTime specializationTimestamp;
  static const flt64_t defaultDepthRate;
  static const ConditionalOperatorEnumType defaultOperator;
};

const arlcore::NumericGuid DepthRateConditionalTest::conditionalId = arlcore::UuidFactory::getInstance().parseGuidFromString("00000000-0000-0000-0000-100000000000");
const arlcore::NumericGuid DepthRateConditionalTest::specializationId = arlcore::UuidFactory::getInstance().parseGuidFromString("00000000-0000-0000-0000-200000000000");
const std::string DepthRateConditionalTest::conditionalName = "TestDepthRateConditional";
const UMAA::Common::Measurement::DateTime DepthRateConditionalTest::specializationTimestamp = UMAA::Common::Measurement::DateTime(1, 100);
const flt64_t DepthRateConditionalTest::defaultDepthRate = 12.34;
const ConditionalOperatorEnumType DepthRateConditionalTest::defaultOperator = ConditionalOperatorEnumType::GREATER_THAN;

TEST_F(DepthRateConditionalTest, testCustomGetters) {
  arlcore::umaa::conditional::DepthRateConditional depthRate(baseConditional_, specializedConditional_);
  EXPECT_EQ(depthRate.getSpecializedConditional(), specializedConditional_);
  EXPECT_EQ(depthRate.getConditionalDepthRate(), defaultDepthRate);
  EXPECT_EQ(depthRate.getConditionalOperatorEnum(), defaultOperator);
  auto deps = depthRate.getDependencies();
  EXPECT_EQ(deps.first, std::nullopt);
  EXPECT_EQ(deps.second, std::nullopt);
}

TEST_F(DepthRateConditionalTest, noVelocityData) {
  arlcore::umaa::conditional::DepthRateConditional depthRate(baseConditional_, specializedConditional_);
  EXPECT_EQ(depthRate.evaluateConditional(), std::nullopt);
}

TEST_F(DepthRateConditionalTest, testGreaterThan) {
  arlcore::umaa::conditional::DepthRateConditional depthRate(baseConditional_, specializedConditional_);
  arlcore::umaa::conditional::VelocityReportType vData;
 
  vData.velocity().downSpeed(12.45);
  depthRate.update(vData);
  ASSERT_TRUE(depthRate.evaluateConditional().has_value());
  EXPECT_TRUE(depthRate.evaluateConditional().value());
  vData.velocity().downSpeed(defaultDepthRate);
  depthRate.update(vData);
  ASSERT_TRUE(depthRate.evaluateConditional().has_value());
  EXPECT_FALSE(depthRate.evaluateConditional().value());
  vData.velocity().downSpeed(2.12);
  depthRate.update(vData);
  ASSERT_TRUE(depthRate.evaluateConditional().has_value());
  EXPECT_FALSE(depthRate.evaluateConditional().value());
}

TEST_F(DepthRateConditionalTest, testGreaterThanOrEqualTo) {
  specializedConditional_.conditionalOp(ConditionalOperatorEnumType::GREATER_THAN_OR_EQUAL_TO);
  arlcore::umaa::conditional::DepthRateConditional depthRate(baseConditional_, specializedConditional_);
  arlcore::umaa::conditional::VelocityReportType vData;
 
  vData.velocity().downSpeed(12.45);
  depthRate.update(vData);
  ASSERT_TRUE(depthRate.evaluateConditional().has_value());
  EXPECT_TRUE(depthRate.evaluateConditional().value());
  vData.velocity().downSpeed(defaultDepthRate);
  depthRate.update(vData);
  ASSERT_TRUE(depthRate.evaluateConditional().has_value());
  EXPECT_TRUE(depthRate.evaluateConditional().value());
  vData.velocity().downSpeed(2.12);
  depthRate.update(vData);
  ASSERT_TRUE(depthRate.evaluateConditional().has_value());
  EXPECT_FALSE(depthRate.evaluateConditional().value());
}

TEST_F(DepthRateConditionalTest, testLessThan) {
  specializedConditional_.conditionalOp(ConditionalOperatorEnumType::LESS_THAN);
  arlcore::umaa::conditional::DepthRateConditional depthRate(baseConditional_, specializedConditional_);
  arlcore::umaa::conditional::VelocityReportType vData;
 
  vData.velocity().downSpeed(12.45);
  depthRate.update(vData);
  ASSERT_TRUE(depthRate.evaluateConditional().has_value());
  EXPECT_FALSE(depthRate.evaluateConditional().value());
  vData.velocity().downSpeed(defaultDepthRate);
  depthRate.update(vData);
  ASSERT_TRUE(depthRate.evaluateConditional().has_value());
  EXPECT_FALSE(depthRate.evaluateConditional().value());
  vData.velocity().downSpeed(2.12);
  depthRate.update(vData);
  ASSERT_TRUE(depthRate.evaluateConditional().has_value());
  EXPECT_TRUE(depthRate.evaluateConditional().value());
}

TEST_F(DepthRateConditionalTest, testLessThanOrEqualTo) {
  specializedConditional_.conditionalOp(ConditionalOperatorEnumType::LESS_THAN_OR_EQUAL_TO);
  arlcore::umaa::conditional::DepthRateConditional depthRate(baseConditional_, specializedConditional_);
  arlcore::umaa::conditional::VelocityReportType vData;
   
  vData.velocity().downSpeed(12.45);
  depthRate.update(vData);
  ASSERT_TRUE(depthRate.evaluateConditional().has_value());
  EXPECT_FALSE(depthRate.evaluateConditional().value());
  vData.velocity().downSpeed(defaultDepthRate);
  depthRate.update(vData);
  ASSERT_TRUE(depthRate.evaluateConditional().has_value());
  EXPECT_TRUE(depthRate.evaluateConditional().value());
  vData.velocity().downSpeed(2.12);
  depthRate.update(vData);
  ASSERT_TRUE(depthRate.evaluateConditional().has_value());
  EXPECT_TRUE(depthRate.evaluateConditional().value());
}