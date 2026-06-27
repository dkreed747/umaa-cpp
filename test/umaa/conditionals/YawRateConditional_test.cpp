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

#include "YawRateConditional.h"
#include "UuidFactory.h"

using UMAA::Common::MaritimeEnumeration::ConditionalOperatorEnumModule::ConditionalOperatorEnumType;

class YawRateConditionalTest : public ::testing::Test {
 protected:
  void SetUp() override {
    baseConditional_ = arlcore::umaa::conditional::ConditionalType(conditionalId, conditionalName, specializationId, specializationTimestamp, arlcore::umaa::conditional::YawRateConditionalTypeTopic);
    specializedConditional_ = arlcore::umaa::conditional::YawRateConditionalType(defaultOperator, defaultYawRate, specializationTimestamp, specializationId);
  }

  void TearDown() override {
    
  }

  arlcore::umaa::conditional::ConditionalType baseConditional_;
  arlcore::umaa::conditional::YawRateConditionalType specializedConditional_;

  static const arlcore::NumericGuid conditionalId;
  static const arlcore::NumericGuid specializationId;
  static const std::string conditionalName;
  static const UMAA::Common::Measurement::DateTime specializationTimestamp;
  static const flt64_t defaultYawRate;
  static const ConditionalOperatorEnumType defaultOperator;
};

const arlcore::NumericGuid YawRateConditionalTest::conditionalId = arlcore::UuidFactory::getInstance().parseGuidFromString("00000000-0000-0000-0000-100000000000");
const arlcore::NumericGuid YawRateConditionalTest::specializationId = arlcore::UuidFactory::getInstance().parseGuidFromString("00000000-0000-0000-0000-200000000000");
const std::string YawRateConditionalTest::conditionalName = "TestYawRateConditional";
const UMAA::Common::Measurement::DateTime YawRateConditionalTest::specializationTimestamp = UMAA::Common::Measurement::DateTime(1, 100);
const flt64_t YawRateConditionalTest::defaultYawRate = 12.34;
const ConditionalOperatorEnumType YawRateConditionalTest::defaultOperator = ConditionalOperatorEnumType::GREATER_THAN;

TEST_F(YawRateConditionalTest, testCustomGetters) {
  arlcore::umaa::conditional::YawRateConditional yawRate(baseConditional_, specializedConditional_);
  EXPECT_EQ(yawRate.getSpecializedConditional(), specializedConditional_);
  EXPECT_EQ(yawRate.getConditionalYawRate(), defaultYawRate);
  EXPECT_EQ(yawRate.getConditionalOperatorEnum(), defaultOperator);
  auto deps = yawRate.getDependencies();
  EXPECT_EQ(deps.first, std::nullopt);
  EXPECT_EQ(deps.second, std::nullopt);
}

TEST_F(YawRateConditionalTest, noVelocityData) {
  arlcore::umaa::conditional::YawRateConditional yawRate(baseConditional_, specializedConditional_);
  EXPECT_EQ(yawRate.evaluateConditional(), std::nullopt);
}

TEST_F(YawRateConditionalTest, testGreaterThan) {
  arlcore::umaa::conditional::YawRateConditional yawRate(baseConditional_, specializedConditional_);
  arlcore::umaa::conditional::VelocityReportType vData;
 
  vData.attitudeRate().yawRate(12.45);
  yawRate.update(vData);
  ASSERT_TRUE(yawRate.evaluateConditional().has_value());
  EXPECT_TRUE(yawRate.evaluateConditional().value());
  vData.attitudeRate().yawRate(defaultYawRate);
  yawRate.update(vData);
  ASSERT_TRUE(yawRate.evaluateConditional().has_value());
  EXPECT_FALSE(yawRate.evaluateConditional().value());
  vData.attitudeRate().yawRate(2.12);
  yawRate.update(vData);
  ASSERT_TRUE(yawRate.evaluateConditional().has_value());
  EXPECT_FALSE(yawRate.evaluateConditional().value());
}

TEST_F(YawRateConditionalTest, testGreaterThanOrEqualTo) {
  specializedConditional_.conditionalOp(ConditionalOperatorEnumType::GREATER_THAN_OR_EQUAL_TO);
  arlcore::umaa::conditional::YawRateConditional yawRate(baseConditional_, specializedConditional_);
  arlcore::umaa::conditional::VelocityReportType vData;
 
  vData.attitudeRate().yawRate(12.45);
  yawRate.update(vData);
  ASSERT_TRUE(yawRate.evaluateConditional().has_value());
  EXPECT_TRUE(yawRate.evaluateConditional().value());
  vData.attitudeRate().yawRate(defaultYawRate);
  yawRate.update(vData);
  ASSERT_TRUE(yawRate.evaluateConditional().has_value());
  EXPECT_TRUE(yawRate.evaluateConditional().value());
  vData.attitudeRate().yawRate(2.12);
  yawRate.update(vData);
  ASSERT_TRUE(yawRate.evaluateConditional().has_value());
  EXPECT_FALSE(yawRate.evaluateConditional().value());
}

TEST_F(YawRateConditionalTest, testLessThan) {
  specializedConditional_.conditionalOp(ConditionalOperatorEnumType::LESS_THAN);
  arlcore::umaa::conditional::YawRateConditional yawRate(baseConditional_, specializedConditional_);
  arlcore::umaa::conditional::VelocityReportType vData;
 
  vData.attitudeRate().yawRate(12.45);
  yawRate.update(vData);
  ASSERT_TRUE(yawRate.evaluateConditional().has_value());
  EXPECT_FALSE(yawRate.evaluateConditional().value());
  vData.attitudeRate().yawRate(defaultYawRate);
  yawRate.update(vData);
  ASSERT_TRUE(yawRate.evaluateConditional().has_value());
  EXPECT_FALSE(yawRate.evaluateConditional().value());
  vData.attitudeRate().yawRate(2.12);
  yawRate.update(vData);
  ASSERT_TRUE(yawRate.evaluateConditional().has_value());
  EXPECT_TRUE(yawRate.evaluateConditional().value());
}

TEST_F(YawRateConditionalTest, testLessThanOrEqualTo) {
  specializedConditional_.conditionalOp(ConditionalOperatorEnumType::LESS_THAN_OR_EQUAL_TO);
  arlcore::umaa::conditional::YawRateConditional yawRate(baseConditional_, specializedConditional_);
  arlcore::umaa::conditional::VelocityReportType vData;
   
  vData.attitudeRate().yawRate(12.45);
  yawRate.update(vData);
  ASSERT_TRUE(yawRate.evaluateConditional().has_value());
  EXPECT_FALSE(yawRate.evaluateConditional().value());
  vData.attitudeRate().yawRate(defaultYawRate);
  yawRate.update(vData);
  ASSERT_TRUE(yawRate.evaluateConditional().has_value());
  EXPECT_TRUE(yawRate.evaluateConditional().value());
  vData.attitudeRate().yawRate(2.12);
  yawRate.update(vData);
  ASSERT_TRUE(yawRate.evaluateConditional().has_value());
  EXPECT_TRUE(yawRate.evaluateConditional().value());
}