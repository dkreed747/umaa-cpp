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

#include <UMAA/MM/Conditional/DepthConditionalType.hpp>

#include "ConditionalBase.h"
#include "UuidFactory.h"
#include "MockConditional.h"

using arlcore::umaa::conditional::ConditionalType;
using UMAA::Common::Measurement::DateTime;

class ConditionalBaseTest : public ::testing::Test {
 protected:

  void SetUp() override {
    MockConditional_ = std::make_shared<arlcore::test::MockConditional>(baseConditional, true);
  }

  void TearDown() override {
    MockConditional_.reset();
  }

  std::shared_ptr<arlcore::test::MockConditional> MockConditional_;

  static const arlcore::NumericGuid conditionalId;
  static const std::string conditionalName;
  static const arlcore::NumericGuid specializationId;
  static const DateTime timestamp;
  static const std::string topic;
  static const ConditionalType baseConditional;
};

const arlcore::NumericGuid ConditionalBaseTest::conditionalId = arlcore::UuidFactory::getInstance().parseGuidFromString("00000000-0000-0000-0000-200000000000");
const std::string ConditionalBaseTest::conditionalName = "test";
const arlcore::NumericGuid ConditionalBaseTest::specializationId = arlcore::UuidFactory::getInstance().parseGuidFromString("00000000-0000-0000-0000-100000000000");
const DateTime ConditionalBaseTest::timestamp = DateTime(1, 100);
const std::string ConditionalBaseTest::topic = "testTopic";
const ConditionalType ConditionalBaseTest::baseConditional = ConditionalType(conditionalId, conditionalName, specializationId, timestamp, topic);

TEST_F(ConditionalBaseTest, testGetters) {
  EXPECT_EQ(MockConditional_->getBaseConditional(), baseConditional);
  EXPECT_EQ(MockConditional_->getConditionalId(), conditionalId);
  EXPECT_EQ(MockConditional_->getName(), conditionalName);
  EXPECT_EQ(MockConditional_->getSpecializationId(), specializationId);
  EXPECT_EQ(MockConditional_->getSpecializationTimestamp(), timestamp);
  EXPECT_EQ(MockConditional_->getSpecializationTopic(), topic);
  auto deps = MockConditional_->getDependencies();
  EXPECT_FALSE(deps.first.has_value());
  EXPECT_FALSE(deps.second.has_value());
  EXPECT_TRUE(MockConditional_->evaluateConditional());
}

TEST_F(ConditionalBaseTest, testSpecializationCheck) {
  UMAA::MM::Conditional::DepthConditionalType specialized(UMAA::Common::MaritimeEnumeration::ConditionalOperatorEnumModule::ConditionalOperatorEnumType::GREATER_THAN,
    0.0, timestamp, specializationId);
  EXPECT_TRUE(arlcore::umaa::conditional::ConditionalBase::isValidSpecialization<UMAA::MM::Conditional::DepthConditionalType>(baseConditional, specialized, topic));
}
