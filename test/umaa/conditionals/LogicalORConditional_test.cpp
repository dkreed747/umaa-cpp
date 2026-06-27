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

#include "LogicalORConditional.h"
#include "UuidFactory.h"
#include "MockConditional.h"

class LogicalORConditionalTest : public ::testing::Test {
 protected:
  void SetUp() override {
    refConditional1_ = std::make_shared<arlcore::test::MockConditional>(refConditionalBase1);
    refConditional2_ = std::make_shared<arlcore::test::MockConditional>(refConditionalBase2);
    orConditional_ = std::make_shared<arlcore::umaa::conditional::LogicalORConditional>(orConditionalBase, orConditionalSpecialization);
    orConditional_->setReferencedConditionals(refConditional1_, refConditional2_);
  }

  void TearDown() override {
    // Code to run after running each TEST_F()
  }

  std::shared_ptr<arlcore::umaa::conditional::LogicalORConditional> orConditional_;
  std::shared_ptr<arlcore::test::MockConditional> refConditional1_;
  std::shared_ptr<arlcore::test::MockConditional> refConditional2_;

  static const arlcore::umaa::conditional::ConditionalType orConditionalBase;
  static const arlcore::umaa::conditional::ConditionalType refConditionalBase1;
  static const arlcore::umaa::conditional::ConditionalType refConditionalBase2;

  static const UMAA::MM::Conditional::LogicalORConditionalType orConditionalSpecialization;

  static const arlcore::NumericGuid refConditionalId1;
  static const arlcore::NumericGuid refConditionalId2;
  static const arlcore::NumericGuid specializationId;

  static const UMAA::Common::Measurement::DateTime timestamp;
  
};

const UMAA::Common::Measurement::DateTime LogicalORConditionalTest::timestamp = UMAA::Common::Measurement::DateTime(0, 0);

const arlcore::NumericGuid LogicalORConditionalTest::refConditionalId1 = arlcore::UuidFactory::getInstance().parseGuidFromString("00000000-0000-0000-0000-100000000000");
const arlcore::NumericGuid LogicalORConditionalTest::refConditionalId2 = arlcore::UuidFactory::getInstance().parseGuidFromString("00000000-0000-0000-0000-200000000000");
const arlcore::NumericGuid LogicalORConditionalTest::specializationId = arlcore::UuidFactory::getInstance().parseGuidFromString("00000000-0000-0000-0000-110000000000");

const UMAA::MM::Conditional::LogicalORConditionalType LogicalORConditionalTest::orConditionalSpecialization = UMAA::MM::Conditional::LogicalORConditionalType(refConditionalId1, refConditionalId2, timestamp, specializationId);

const arlcore::umaa::conditional::ConditionalType LogicalORConditionalTest::orConditionalBase = arlcore::umaa::conditional::ConditionalType(specializationId, "ORConditional", specializationId, timestamp, UMAA::MM::Conditional::LogicalORConditionalTypeTopic);
const arlcore::umaa::conditional::ConditionalType LogicalORConditionalTest::refConditionalBase1 = arlcore::umaa::conditional::ConditionalType(refConditionalId1, "Ref1", specializationId, timestamp, "TestTopic");
const arlcore::umaa::conditional::ConditionalType LogicalORConditionalTest::refConditionalBase2 = arlcore::umaa::conditional::ConditionalType(refConditionalId2, "Ref2", specializationId, timestamp, "TestTopic");


TEST_F(LogicalORConditionalTest, testCustomGetters) {
  EXPECT_EQ(orConditional_->getSpecializedConditional(), orConditionalSpecialization);
}

TEST_F(LogicalORConditionalTest, testNullptrs) {
  std::shared_ptr<arlcore::umaa::conditional::ConditionalBase> ptr(nullptr);
  orConditional_->setReferencedConditionals(ptr, ptr);
  EXPECT_EQ(orConditional_->evaluateConditional(), std::nullopt);
}

TEST_F(LogicalORConditionalTest, testEvaluations) {
  // false, false
  EXPECT_TRUE(orConditional_->evaluateConditional().has_value());
  EXPECT_FALSE(orConditional_->evaluateConditional().value());

  // true, false
  refConditional1_->setEvaluationValue(true);
  EXPECT_TRUE(orConditional_->evaluateConditional().has_value());
  EXPECT_TRUE(orConditional_->evaluateConditional().value());

  // false, true
  refConditional1_->setEvaluationValue(false);
  refConditional2_->setEvaluationValue(true);
  EXPECT_TRUE(orConditional_->evaluateConditional().has_value());
  EXPECT_TRUE(orConditional_->evaluateConditional().value());

  // true, true
  refConditional1_->setEvaluationValue(true);
  EXPECT_TRUE(orConditional_->evaluateConditional().has_value());
  EXPECT_TRUE(orConditional_->evaluateConditional().value());
}