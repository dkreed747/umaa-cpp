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

#include "DepthConditional.h"
#include "UuidFactory.h"

using UMAA::Common::MaritimeEnumeration::ConditionalOperatorEnumModule::ConditionalOperatorEnumType;

class DepthConditionalTest : public ::testing::Test {
 protected:
  void SetUp() override {
    baseConditional_ = arlcore::umaa::conditional::ConditionalType(conditionalId, conditionalName, specializationId, specializationTimestamp, arlcore::umaa::conditional::DepthConditionalTypeTopic);
    specializedConditional_ = arlcore::umaa::conditional::DepthConditionalType(defaultOperator, defaultDepth, specializationTimestamp, specializationId);
  }

  void TearDown() override {
    
  }

  arlcore::umaa::conditional::ConditionalType baseConditional_;
  arlcore::umaa::conditional::DepthConditionalType specializedConditional_;

  static const arlcore::NumericGuid conditionalId;
  static const arlcore::NumericGuid specializationId;
  static const std::string conditionalName;
  static const UMAA::Common::Measurement::DateTime specializationTimestamp;
  static const flt64_t defaultDepth;
  static const ConditionalOperatorEnumType defaultOperator;
};

const arlcore::NumericGuid DepthConditionalTest::conditionalId = arlcore::UuidFactory::getInstance().parseGuidFromString("00000000-0000-0000-0000-100000000000");
const arlcore::NumericGuid DepthConditionalTest::specializationId = arlcore::UuidFactory::getInstance().parseGuidFromString("00000000-0000-0000-0000-200000000000");
const std::string DepthConditionalTest::conditionalName = "TestDepthConditional";
const UMAA::Common::Measurement::DateTime DepthConditionalTest::specializationTimestamp = UMAA::Common::Measurement::DateTime(1, 100);
const flt64_t DepthConditionalTest::defaultDepth = 12.34;
const ConditionalOperatorEnumType DepthConditionalTest::defaultOperator = ConditionalOperatorEnumType::GREATER_THAN;

TEST_F(DepthConditionalTest, testCustomGetters) {
  arlcore::umaa::conditional::DepthConditional depth(baseConditional_, specializedConditional_);
  EXPECT_EQ(depth.getSpecializedConditional(), specializedConditional_);
  EXPECT_EQ(depth.getConditionalDepth(), defaultDepth);
  EXPECT_EQ(depth.getConditionalOperatorEnum(), defaultOperator);
  auto deps = depth.getDependencies();
  EXPECT_EQ(deps.first, std::nullopt);
  EXPECT_EQ(deps.second, std::nullopt);
}

TEST_F(DepthConditionalTest, noGlobalPoseData) {
  arlcore::umaa::conditional::DepthConditional depth(baseConditional_, specializedConditional_);
  EXPECT_EQ(depth.evaluateConditional(), std::nullopt);
}

TEST_F(DepthConditionalTest, testGreaterThan) {
  arlcore::umaa::conditional::DepthConditional depth(baseConditional_, specializedConditional_);
  arlcore::umaa::conditional::GlobalPoseReportType gpData;

  gpData.depth(12.45);
  depth.update(gpData);
  ASSERT_TRUE(depth.evaluateConditional().has_value());
  EXPECT_TRUE(depth.evaluateConditional().value());
  gpData.depth(defaultDepth);
  depth.update(gpData);
  ASSERT_TRUE(depth.evaluateConditional().has_value());
  EXPECT_FALSE(depth.evaluateConditional().value());
  gpData.depth(2.12);
  depth.update(gpData);
  ASSERT_TRUE(depth.evaluateConditional().has_value());
  EXPECT_FALSE(depth.evaluateConditional().value());
}

TEST_F(DepthConditionalTest, testGreaterThanOrEqualTo) {
  specializedConditional_.conditionalOp(ConditionalOperatorEnumType::GREATER_THAN_OR_EQUAL_TO);
  arlcore::umaa::conditional::DepthConditional depth(baseConditional_, specializedConditional_);
  arlcore::umaa::conditional::GlobalPoseReportType gpData;
  
  gpData.depth(12.45);
  depth.update(gpData);
  ASSERT_TRUE(depth.evaluateConditional().has_value());
  EXPECT_TRUE(depth.evaluateConditional().value());
  gpData.depth(defaultDepth);
  depth.update(gpData);
  ASSERT_TRUE(depth.evaluateConditional().has_value());
  EXPECT_TRUE(depth.evaluateConditional().value());
  gpData.depth(2.12);
  depth.update(gpData);
  ASSERT_TRUE(depth.evaluateConditional().has_value());
  EXPECT_FALSE(depth.evaluateConditional().value());
}

TEST_F(DepthConditionalTest, testLessThan) {
  specializedConditional_.conditionalOp(ConditionalOperatorEnumType::LESS_THAN);
  arlcore::umaa::conditional::DepthConditional depth(baseConditional_, specializedConditional_);
  arlcore::umaa::conditional::GlobalPoseReportType gpData;
  
  gpData.depth(12.45);
  depth.update(gpData);
  ASSERT_TRUE(depth.evaluateConditional().has_value());
  EXPECT_FALSE(depth.evaluateConditional().value());
  gpData.depth(defaultDepth);
  depth.update(gpData);
  ASSERT_TRUE(depth.evaluateConditional().has_value());
  EXPECT_FALSE(depth.evaluateConditional().value());
  gpData.depth(2.12);
  depth.update(gpData);
  ASSERT_TRUE(depth.evaluateConditional().has_value());
  EXPECT_TRUE(depth.evaluateConditional().value());
}

TEST_F(DepthConditionalTest, testLessThanOrEqualTo) {
  specializedConditional_.conditionalOp(ConditionalOperatorEnumType::LESS_THAN_OR_EQUAL_TO);
  arlcore::umaa::conditional::DepthConditional depth(baseConditional_, specializedConditional_);
  arlcore::umaa::conditional::GlobalPoseReportType gpData;
    
  gpData.depth(12.45);
  depth.update(gpData);
  ASSERT_TRUE(depth.evaluateConditional().has_value());
  EXPECT_FALSE(depth.evaluateConditional().value());
  gpData.depth(defaultDepth);
  depth.update(gpData);
  ASSERT_TRUE(depth.evaluateConditional().has_value());
  EXPECT_TRUE(depth.evaluateConditional().value());
  gpData.depth(2.12);
  depth.update(gpData);
  ASSERT_TRUE(depth.evaluateConditional().has_value());
  EXPECT_TRUE(depth.evaluateConditional().value());
}