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

#include <UMAA/MM/Conditional/ConditionalType.hpp>
#include <UMAA/MM/Conditional/DepthConditionalType.hpp>

#include "SpecializationCache.h"
#include "LocalReaderSender.h"
#include "UuidFactory.h"

using UMAA::MM::Conditional::DepthConditionalType;

class SpecializationCacheTest : public ::testing::Test {
 protected:
  static void SetUpTestSuite() {
    reader_ = std::make_shared<arlcore::io::LocalReaderSender<DepthConditionalType>>();
  }

  void SetUp() override {

  }

  void TearDown() override {
    reader_->clear();
  }

  static const arlcore::NumericGuid goodSpecializationId_;
static const arlcore::NumericGuid badSpecializationId_;


  static std::shared_ptr<arlcore::io::LocalReaderSender<DepthConditionalType>> reader_;
};

const arlcore::NumericGuid SpecializationCacheTest::goodSpecializationId_ =
  arlcore::UuidFactory::getInstance().parseGuidFromString("00000000-0000-0000-0000-100000000000");
const arlcore::NumericGuid SpecializationCacheTest::badSpecializationId_ =
  arlcore::UuidFactory::getInstance().parseGuidFromString("00000000-0000-0000-0000-200000000000");

std::shared_ptr<arlcore::io::LocalReaderSender<DepthConditionalType>> SpecializationCacheTest::reader_;

TEST_F(SpecializationCacheTest, getSpecialization) {
  arlcore::umaa::SpecializationCache<DepthConditionalType> cache(reader_);

  DepthConditionalType conditional;
  conditional.conditionalOp(UMAA::Common::MaritimeEnumeration::ConditionalOperatorEnumModule::ConditionalOperatorEnumType::GREATER_THAN);
  conditional.depth(1.234);
  conditional.specializationReferenceID(goodSpecializationId_.getGuid());
  conditional.specializationReferenceTimestamp(UMAA::Common::Measurement::DateTime(0, 0));
  reader_->send(conditional);

  cache.update();

  auto result = cache.getSpecializationByRefId(goodSpecializationId_);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->depth(), conditional.depth());

  result = cache.getSpecializationByRefId(goodSpecializationId_, UMAA::Common::Measurement::DateTime(1, 0));
  EXPECT_FALSE(result.has_value());

  UMAA::MM::Conditional::ConditionalType base;
  base.specializationID(goodSpecializationId_);
  base.specializationTimestamp(UMAA::Common::Measurement::DateTime(0, 0));
  result = cache.getSpecialization<UMAA::MM::Conditional::ConditionalType>(base);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->depth(), conditional.depth());

  base.specializationTimestamp(UMAA::Common::Measurement::DateTime(1, 0));
  result = cache.getSpecialization<UMAA::MM::Conditional::ConditionalType>(base);
  EXPECT_FALSE(result.has_value());

  result = cache.getSpecializationByRefId(badSpecializationId_);
  EXPECT_FALSE(result.has_value());

}

TEST_F(SpecializationCacheTest, updateExisting) {
  arlcore::umaa::SpecializationCache<DepthConditionalType> cache(reader_);

  DepthConditionalType conditional;
  conditional.conditionalOp(UMAA::Common::MaritimeEnumeration::ConditionalOperatorEnumModule::ConditionalOperatorEnumType::GREATER_THAN);
  conditional.depth(1.234);
  conditional.specializationReferenceID(goodSpecializationId_.getGuid());
  conditional.specializationReferenceTimestamp(UMAA::Common::Measurement::DateTime(0, 0));
  reader_->send(conditional);
  conditional.depth(2.468);
  conditional.specializationReferenceTimestamp(UMAA::Common::Measurement::DateTime(1, 0));
  reader_->send(conditional);

  cache.update();

  auto result = cache.getSpecializationByRefId(goodSpecializationId_);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->depth(), conditional.depth());
  EXPECT_EQ(result->specializationReferenceTimestamp(), conditional.specializationReferenceTimestamp());

  result = cache.getSpecializationByRefId(badSpecializationId_);
  EXPECT_FALSE(result.has_value());

}
