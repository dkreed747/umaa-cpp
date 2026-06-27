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

#include "HeadingSectorConditional.h"
#include "UuidFactory.h"

using arlcore::umaa::conditional::HeadingSectorKindEnumType;
using UMAA::MM::Conditional::HeadingSectorType;

class HeadingSectorConditionalTest : public ::testing::Test {
 protected:
  void SetUp() override {
    baseConditional_ = arlcore::umaa::conditional::ConditionalType(conditionalId, conditionalName, specializationId, specializationTimestamp, arlcore::umaa::conditional::HeadingSectorConditionalTypeTopic);
    specializedConditional_ = arlcore::umaa::conditional::HeadingSectorConditionalType(defaultHeadingSector, specializationTimestamp, specializationId);
  }

  void TearDown() override {
    
  }

  arlcore::umaa::conditional::ConditionalType baseConditional_;
  arlcore::umaa::conditional::HeadingSectorConditionalType specializedConditional_;

  static const arlcore::NumericGuid conditionalId;
  static const arlcore::NumericGuid specializationId;
  static const std::string conditionalName;
  static const UMAA::Common::Measurement::DateTime specializationTimestamp;
  static const std::array<HeadingSectorType, 2L> sectors;
  static const ::std::vector<HeadingSectorType> defaultHeadingSector;
};

const arlcore::NumericGuid HeadingSectorConditionalTest::conditionalId = arlcore::UuidFactory::getInstance().parseGuidFromString("00000000-0000-0000-0000-100000000000");
const arlcore::NumericGuid HeadingSectorConditionalTest::specializationId = arlcore::UuidFactory::getInstance().parseGuidFromString("00000000-0000-0000-0000-200000000000");
const std::string HeadingSectorConditionalTest::conditionalName = "TestHeadingSectorConditional";
const UMAA::Common::Measurement::DateTime HeadingSectorConditionalTest::specializationTimestamp = UMAA::Common::Measurement::DateTime(1, 100);
const std::array<HeadingSectorType, 2L> HeadingSectorConditionalTest::sectors = {HeadingSectorType(3, HeadingSectorKindEnumType::INSIDE, -3), HeadingSectorType(1, HeadingSectorKindEnumType::OUTSIDE, -1)};
const ::std::vector<HeadingSectorType> HeadingSectorConditionalTest::defaultHeadingSector = ::std::vector<HeadingSectorType>(sectors.begin(), sectors.end());

TEST_F(HeadingSectorConditionalTest, testCustomGetters) {
  arlcore::umaa::conditional::HeadingSectorConditional headingSector(baseConditional_, specializedConditional_);
  EXPECT_EQ(headingSector.getSpecializedConditional(), specializedConditional_);
  auto sectors = headingSector.getConditionalSectors();
  for (int i = 0; i < defaultHeadingSector.size(); ++i) {
    EXPECT_EQ(sectors.at(i), defaultHeadingSector.at(i));
  }
  auto deps = headingSector.getDependencies();
  EXPECT_EQ(deps.first, std::nullopt);
  EXPECT_EQ(deps.second, std::nullopt);
}

TEST_F(HeadingSectorConditionalTest, noGlobalPoseData) {
  arlcore::umaa::conditional::HeadingSectorConditional headingSector(baseConditional_, specializedConditional_);
  EXPECT_EQ(headingSector.evaluateConditional(), std::nullopt);
}

TEST_F(HeadingSectorConditionalTest, testSectors) {
  arlcore::umaa::conditional::HeadingSectorConditional headingSector(baseConditional_, specializedConditional_);
  arlcore::umaa::conditional::GlobalPoseReportType gpData;

  // Outside first sector, false
  gpData.attitude().yaw().yaw(-4);
  headingSector.update(gpData);
  EXPECT_TRUE(headingSector.evaluateConditional().has_value());
  EXPECT_FALSE(headingSector.evaluateConditional().value());

  // Inside second sector, false
  gpData.attitude().yaw().yaw(0);
  headingSector.update(gpData);
  EXPECT_TRUE(headingSector.evaluateConditional().has_value());
  EXPECT_FALSE(headingSector.evaluateConditional().value());

  // Inside first sector, outside second sector, true
  gpData.attitude().yaw().yaw(2);
  headingSector.update(gpData);
  EXPECT_TRUE(headingSector.evaluateConditional().has_value());
  EXPECT_TRUE(headingSector.evaluateConditional().value());
}
