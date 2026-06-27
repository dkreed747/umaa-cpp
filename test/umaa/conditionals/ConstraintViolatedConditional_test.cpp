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
#include <thread>

#include "ConstraintViolatedConditional.h"
#include "UuidFactory.h"
#include "MockConditional.h"

class ConstraintViolatedConditionalTest : public ::testing::Test {
 protected:
  void SetUp() override {
    constraintViolatedConditionalSpecialization_.constraintConditionalID(constraintConditionalId);
    constraintViolatedConditionalSpecialization_.specializationReferenceID(specializationId);
    constraintViolatedConditionalSpecialization_.specializationReferenceTimestamp(specializationTimestamp);
    constraintConditional_ = std::make_shared<arlcore::test::MockConditional>(constraintConditionalBase, true);
  }

  void TearDown() override {
    constraintConditional_.reset();
  }

  std::shared_ptr<arlcore::test::MockConditional> constraintConditional_;
  UMAA::MM::Conditional::ConstraintViolatedConditionalType constraintViolatedConditionalSpecialization_;

  static const arlcore::umaa::conditional::ConditionalType baseConditional;
  static const arlcore::umaa::conditional::ConditionalType constraintConditionalBase;

  static const arlcore::NumericGuid conditionalId;
  static const arlcore::NumericGuid constraintConditionalId;
  static const arlcore::NumericGuid specializationId;
  static const UMAA::Common::Measurement::DateTime specializationTimestamp;
};

const arlcore::NumericGuid ConstraintViolatedConditionalTest::conditionalId = arlcore::UuidFactory::getInstance().parseGuidFromString("00000000-0000-0000-0000-100000000000");
const arlcore::NumericGuid ConstraintViolatedConditionalTest::constraintConditionalId = arlcore::UuidFactory::getInstance().parseGuidFromString("00000000-0000-0000-0000-200000000000");
const arlcore::NumericGuid ConstraintViolatedConditionalTest::specializationId = arlcore::UuidFactory::getInstance().parseGuidFromString("00000000-0000-0000-0000-300000000000");
const UMAA::Common::Measurement::DateTime ConstraintViolatedConditionalTest::specializationTimestamp = UMAA::Common::Measurement::DateTime(1, 100);

const arlcore::umaa::conditional::ConditionalType ConstraintViolatedConditionalTest::baseConditional = arlcore::umaa::conditional::ConditionalType(conditionalId, "ConstraintViolatedConditional", specializationId, specializationTimestamp, arlcore::umaa::conditional::ConstraintViolatedConditionalTypeTopic);
const arlcore::umaa::conditional::ConditionalType ConstraintViolatedConditionalTest::constraintConditionalBase = arlcore::umaa::conditional::ConditionalType(constraintConditionalId, "ConstraintConditional", specializationId, specializationTimestamp, "TestTopic");

TEST_F(ConstraintViolatedConditionalTest, testNullptr) {
  arlcore::umaa::conditional::ConstraintViolatedConditional conditional(baseConditional, constraintViolatedConditionalSpecialization_);

  EXPECT_EQ(conditional.evaluateConditional(), std::nullopt);
}

TEST_F(ConstraintViolatedConditionalTest, testLinkMismatched) {
  arlcore::umaa::conditional::ConstraintViolatedConditional conditional(baseConditional, constraintViolatedConditionalSpecialization_);
  arlcore::umaa::conditional::ConditionalType mismatchBase(arlcore::NIL_GUID, "ConstraintConditional", specializationId, specializationTimestamp, "TestTopic");
  auto mismatch = std::make_shared<arlcore::test::MockConditional>(mismatchBase, true);

  conditional.setConstraintConditional(mismatch);
  EXPECT_EQ(conditional.evaluateConditional(), std::nullopt);
}

TEST_F(ConstraintViolatedConditionalTest, testGetSpecialization) {
  arlcore::umaa::conditional::ConstraintViolatedConditional conditional(baseConditional, constraintViolatedConditionalSpecialization_);
  EXPECT_EQ(conditional.getSpecializedConditional(), constraintViolatedConditionalSpecialization_);
}

TEST_F(ConstraintViolatedConditionalTest, testNoDuration) {
  constraintViolatedConditionalSpecialization_.duration().reset();
  arlcore::umaa::conditional::ConstraintViolatedConditional conditional(baseConditional, constraintViolatedConditionalSpecialization_);
  conditional.setConstraintConditional(constraintConditional_);

  // constraint true
  EXPECT_TRUE(conditional.evaluateConditional().has_value());
  EXPECT_FALSE(conditional.evaluateConditional().value());

  // constraint false
  constraintConditional_->setEvaluationValue(false);
  EXPECT_TRUE(conditional.evaluateConditional().has_value());
  EXPECT_TRUE(conditional.evaluateConditional().value());
}

TEST_F(ConstraintViolatedConditionalTest, testDuration) {
  // Set duration to 1ms for test
  constraintViolatedConditionalSpecialization_.duration(0.001);
  arlcore::umaa::conditional::ConstraintViolatedConditional conditional(baseConditional, constraintViolatedConditionalSpecialization_);
  conditional.setConstraintConditional(constraintConditional_);

  // constraint true
  EXPECT_TRUE(conditional.evaluateConditional().has_value());
  EXPECT_FALSE(conditional.evaluateConditional().value());

  // constraint false, no immediate change
  constraintConditional_->setEvaluationValue(false);
  EXPECT_TRUE(conditional.evaluateConditional().has_value());
  EXPECT_FALSE(conditional.evaluateConditional().value());

  // violated after 1 second
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  EXPECT_TRUE(conditional.evaluateConditional().has_value());
  EXPECT_TRUE(conditional.evaluateConditional().value());

  // resets after constraint updated
  constraintConditional_->setEvaluationValue(true);
  EXPECT_TRUE(conditional.evaluateConditional().has_value());
  EXPECT_FALSE(conditional.evaluateConditional().value());
}