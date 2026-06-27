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

#include "TimeConditional.h"
#include "UuidFactory.h"

using UMAA::Common::MaritimeEnumeration::ConditionalOperatorEnumModule::ConditionalOperatorEnumType;

class TimeConditionalTest : public ::testing::Test {
 protected:
  void SetUp() override {
    auto time = clock.getCurrentTime_timestamp();
    startTime_.seconds(time.seconds);
    startTime_.nanoseconds(time.nanoseconds);
    baseConditional_ = arlcore::umaa::conditional::ConditionalType(conditionalId, conditionalName, specializationId, specializationTimestamp, arlcore::umaa::conditional::TimeConditionalTypeTopic);
    specializedConditional_ = arlcore::umaa::conditional::TimeConditionalType(defaultOperator, startTime_, specializationTimestamp, specializationId);
  }

  void TearDown() override {
    
  }

  arlcore::umaa::conditional::ConditionalType baseConditional_;
  arlcore::umaa::conditional::TimeConditionalType specializedConditional_;
  DateTime startTime_;

  static const arlcore::NumericGuid conditionalId;
  static const arlcore::NumericGuid specializationId;
  static const std::string conditionalName;
  static const UMAA::Common::Measurement::DateTime specializationTimestamp;
  static const UMAA::Common::Measurement::DateTime defaultTime;
  static const ConditionalOperatorEnumType defaultOperator;
  static const arlcore::RealtimeSystemClock clock;
};

const arlcore::NumericGuid TimeConditionalTest::conditionalId = arlcore::UuidFactory::getInstance().parseGuidFromString("00000000-0000-0000-0000-100000000000");
const arlcore::NumericGuid TimeConditionalTest::specializationId = arlcore::UuidFactory::getInstance().parseGuidFromString("00000000-0000-0000-0000-200000000000");
const std::string TimeConditionalTest::conditionalName = "TestTimeConditional";
const UMAA::Common::Measurement::DateTime TimeConditionalTest::specializationTimestamp = UMAA::Common::Measurement::DateTime(1, 100);
const ConditionalOperatorEnumType TimeConditionalTest::defaultOperator = ConditionalOperatorEnumType::GREATER_THAN;
const arlcore::RealtimeSystemClock TimeConditionalTest::clock;

TEST_F(TimeConditionalTest, testCustomGetters) {
  arlcore::umaa::conditional::TimeConditional timeConditional(baseConditional_, specializedConditional_);
  EXPECT_EQ(timeConditional.getSpecializedConditional(), specializedConditional_);
  EXPECT_EQ(timeConditional.getConditionalTime(), startTime_);
  EXPECT_EQ(timeConditional.getConditionalOperatorEnum(), defaultOperator);
  auto deps = timeConditional.getDependencies();
  EXPECT_EQ(deps.first, std::nullopt);
  EXPECT_EQ(deps.second, std::nullopt);
}

TEST_F(TimeConditionalTest, testGreaterThan) {
  arlcore::umaa::conditional::TimeConditional timeConditional(baseConditional_, specializedConditional_);
  ASSERT_TRUE(timeConditional.evaluateConditional().has_value());
  EXPECT_TRUE(timeConditional.evaluateConditional().value());
  specializedConditional_.time().seconds(specializedConditional_.time().seconds() + 10);
  timeConditional = arlcore::umaa::conditional::TimeConditional(baseConditional_, specializedConditional_);
  ASSERT_TRUE(timeConditional.evaluateConditional().has_value());
  EXPECT_FALSE(timeConditional.evaluateConditional().value());
}

TEST_F(TimeConditionalTest, testGreaterThanOrEqualTo) {
  specializedConditional_.conditionalOp(ConditionalOperatorEnumType::GREATER_THAN_OR_EQUAL_TO);
  arlcore::umaa::conditional::TimeConditional timeConditional(baseConditional_, specializedConditional_);
  ASSERT_TRUE(timeConditional.evaluateConditional().has_value());
  EXPECT_TRUE(timeConditional.evaluateConditional().value());
  specializedConditional_.time().seconds(specializedConditional_.time().seconds() + 10);
  timeConditional = arlcore::umaa::conditional::TimeConditional(baseConditional_, specializedConditional_);
  ASSERT_TRUE(timeConditional.evaluateConditional().has_value());
  EXPECT_FALSE(timeConditional.evaluateConditional().value());
}

TEST_F(TimeConditionalTest, testLessThan) {
  specializedConditional_.conditionalOp(ConditionalOperatorEnumType::LESS_THAN);
  arlcore::umaa::conditional::TimeConditional timeConditional(baseConditional_, specializedConditional_);
  ASSERT_TRUE(timeConditional.evaluateConditional().has_value());
  EXPECT_FALSE(timeConditional.evaluateConditional().value());
  specializedConditional_.time().seconds(specializedConditional_.time().seconds() + 10);
  timeConditional = arlcore::umaa::conditional::TimeConditional(baseConditional_, specializedConditional_);
  ASSERT_TRUE(timeConditional.evaluateConditional().has_value());
  EXPECT_TRUE(timeConditional.evaluateConditional().value());
}

TEST_F(TimeConditionalTest, testLessThanOrEqualTo) {
  specializedConditional_.conditionalOp(ConditionalOperatorEnumType::LESS_THAN_OR_EQUAL_TO);
  arlcore::umaa::conditional::TimeConditional timeConditional(baseConditional_, specializedConditional_);
  ASSERT_TRUE(timeConditional.evaluateConditional().has_value());
  EXPECT_FALSE(timeConditional.evaluateConditional().value());
  specializedConditional_.time().seconds(specializedConditional_.time().seconds() + 10);
  timeConditional = arlcore::umaa::conditional::TimeConditional(baseConditional_, specializedConditional_);
  ASSERT_TRUE(timeConditional.evaluateConditional().has_value());
  EXPECT_TRUE(timeConditional.evaluateConditional().value());
}