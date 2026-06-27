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

#include "LogicalNOTConditional.h"
#include "UuidFactory.h"
#include "MockConditional.h"

class LogicalNOTConditionalTest : public ::testing::Test {
 protected:
  void SetUp() override {
    negatedConditional_ = std::make_shared<arlcore::test::MockConditional>(negatedConditionalBase);
    notConditional_ = std::make_shared<arlcore::umaa::conditional::LogicalNOTConditional>(notConditionalBase, notConditionalSpecialization);
    notConditional_->setNegatedConditional(negatedConditional_);
  }

  void TearDown() override {
    // Code to run after running each TEST_F()
  }

  std::shared_ptr<arlcore::umaa::conditional::LogicalNOTConditional> notConditional_;
  std::shared_ptr<arlcore::test::MockConditional> negatedConditional_;

  static const arlcore::umaa::conditional::ConditionalType notConditionalBase;
  static const arlcore::umaa::conditional::ConditionalType negatedConditionalBase;

  static const UMAA::MM::Conditional::LogicalNOTConditionalType notConditionalSpecialization;

  static const arlcore::NumericGuid negatedConditionalId;
  static const arlcore::NumericGuid specializationId;

  static const UMAA::Common::Measurement::DateTime timestamp;
  
};

const UMAA::Common::Measurement::DateTime LogicalNOTConditionalTest::timestamp = UMAA::Common::Measurement::DateTime(0, 0);

const arlcore::NumericGuid LogicalNOTConditionalTest::negatedConditionalId = arlcore::UuidFactory::getInstance().parseGuidFromString("00000000-0000-0000-0000-100000000000");
const arlcore::NumericGuid LogicalNOTConditionalTest::specializationId = arlcore::UuidFactory::getInstance().parseGuidFromString("00000000-0000-0000-0000-110000000000");

const UMAA::MM::Conditional::LogicalNOTConditionalType LogicalNOTConditionalTest::notConditionalSpecialization = UMAA::MM::Conditional::LogicalNOTConditionalType(negatedConditionalId, timestamp, specializationId);

const arlcore::umaa::conditional::ConditionalType LogicalNOTConditionalTest::notConditionalBase = arlcore::umaa::conditional::ConditionalType(specializationId, "NotConditional", specializationId, timestamp, UMAA::MM::Conditional::LogicalNOTConditionalTypeTopic);
const arlcore::umaa::conditional::ConditionalType LogicalNOTConditionalTest::negatedConditionalBase = arlcore::umaa::conditional::ConditionalType(negatedConditionalId, "NegatedConditioanl", specializationId, timestamp, "TestTopic");


TEST_F(LogicalNOTConditionalTest, testCustomGetters) {
  EXPECT_EQ(notConditional_->getSpecializedConditional(), notConditionalSpecialization);
}

TEST_F(LogicalNOTConditionalTest, testNullptrs) {
  std::shared_ptr<arlcore::umaa::conditional::ConditionalBase> ptr(nullptr);
  notConditional_->setNegatedConditional(ptr);
  EXPECT_EQ(notConditional_->evaluateConditional(), std::nullopt);
}

TEST_F(LogicalNOTConditionalTest, testEvaluations) {
  // false
  EXPECT_TRUE(notConditional_->evaluateConditional().has_value());
  EXPECT_TRUE(notConditional_->evaluateConditional().value());

  // true
  negatedConditional_->setEvaluationValue(true);
  EXPECT_TRUE(notConditional_->evaluateConditional().has_value());
  EXPECT_FALSE(notConditional_->evaluateConditional().value());
}