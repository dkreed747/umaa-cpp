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

#include "LogicalANDConditional.h"
#include "UuidFactory.h"
#include "MockConditional.h"

class LogicalANDConditionalTest : public ::testing::Test {
 protected:
  void SetUp() override {
    refConditional1_ = std::make_shared<arlcore::test::MockConditional>(refConditionalBase1);
    refConditional2_ = std::make_shared<arlcore::test::MockConditional>(refConditionalBase2);
    andConditional_ = std::make_shared<arlcore::umaa::conditional::LogicalANDConditional>(andConditionalBase, andConditionalSpecialization);
    andConditional_->setReferencedConditionals(refConditional1_, refConditional2_);
  }

  void TearDown() override {
    // Code to run after running each TEST_F()
  }

  std::shared_ptr<arlcore::umaa::conditional::LogicalANDConditional> andConditional_;
  std::shared_ptr<arlcore::test::MockConditional> refConditional1_;
  std::shared_ptr<arlcore::test::MockConditional> refConditional2_;

  static const arlcore::umaa::conditional::ConditionalType andConditionalBase;
  static const arlcore::umaa::conditional::ConditionalType refConditionalBase1;
  static const arlcore::umaa::conditional::ConditionalType refConditionalBase2;

  static const UMAA::MM::Conditional::LogicalANDConditionalType andConditionalSpecialization;

  static const arlcore::NumericGuid refConditionalId1;
  static const arlcore::NumericGuid refConditionalId2;
  static const arlcore::NumericGuid specializationId;

  static const UMAA::Common::Measurement::DateTime timestamp;
  
};

const UMAA::Common::Measurement::DateTime LogicalANDConditionalTest::timestamp = UMAA::Common::Measurement::DateTime(0, 0);

const arlcore::NumericGuid LogicalANDConditionalTest::refConditionalId1 = arlcore::UuidFactory::getInstance().parseGuidFromString("00000000-0000-0000-0000-100000000000");
const arlcore::NumericGuid LogicalANDConditionalTest::refConditionalId2 = arlcore::UuidFactory::getInstance().parseGuidFromString("00000000-0000-0000-0000-200000000000");
const arlcore::NumericGuid LogicalANDConditionalTest::specializationId = arlcore::UuidFactory::getInstance().parseGuidFromString("00000000-0000-0000-0000-110000000000");

const UMAA::MM::Conditional::LogicalANDConditionalType LogicalANDConditionalTest::andConditionalSpecialization = UMAA::MM::Conditional::LogicalANDConditionalType(refConditionalId1, refConditionalId2, timestamp, specializationId);

const arlcore::umaa::conditional::ConditionalType LogicalANDConditionalTest::andConditionalBase = arlcore::umaa::conditional::ConditionalType(specializationId, "ANDConditional", specializationId, timestamp, UMAA::MM::Conditional::LogicalANDConditionalTypeTopic);
const arlcore::umaa::conditional::ConditionalType LogicalANDConditionalTest::refConditionalBase1 = arlcore::umaa::conditional::ConditionalType(refConditionalId1, "Ref1", specializationId, timestamp, "TestTopic");
const arlcore::umaa::conditional::ConditionalType LogicalANDConditionalTest::refConditionalBase2 = arlcore::umaa::conditional::ConditionalType(refConditionalId2, "Ref2", specializationId, timestamp, "TestTopic");

TEST_F(LogicalANDConditionalTest, testCustomGetters) {
  EXPECT_EQ(andConditional_->getSpecializedConditional(), andConditionalSpecialization);
}

TEST_F(LogicalANDConditionalTest, testNullptrs) {
  std::shared_ptr<arlcore::umaa::conditional::ConditionalBase> ptr(nullptr);
  andConditional_->setReferencedConditionals(ptr, ptr);
  EXPECT_EQ(andConditional_->evaluateConditional(), std::nullopt);
}

TEST_F(LogicalANDConditionalTest, testEvaluations) {
  // false, false
  EXPECT_TRUE(andConditional_->evaluateConditional().has_value());
  EXPECT_FALSE(andConditional_->evaluateConditional().value());

  // true, false
  refConditional1_->setEvaluationValue(true);
  EXPECT_TRUE(andConditional_->evaluateConditional().has_value());
  EXPECT_FALSE(andConditional_->evaluateConditional().value());

  // false, true
  refConditional1_->setEvaluationValue(false);
  refConditional2_->setEvaluationValue(true);
  EXPECT_TRUE(andConditional_->evaluateConditional().has_value());
  EXPECT_FALSE(andConditional_->evaluateConditional().value());

  // true, true
  refConditional1_->setEvaluationValue(true);
  EXPECT_TRUE(andConditional_->evaluateConditional().has_value());
  EXPECT_TRUE(andConditional_->evaluateConditional().value());
}