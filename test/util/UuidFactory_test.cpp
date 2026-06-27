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
#include <uuid/uuid.h>

#include "NumericGuid.h"
#include "UuidFactory.h"

TEST(UuidFactory_test, parseGuidFromString)
{
  std::string inputUuid("20929ee2-db05-48f8-9f8a-700869db8091");
  arlcore::UuidFactory& factory = arlcore::UuidFactory::getInstance();

  // Convert the string guid into a NumericGuid object
  arlcore::NumericGuid testGuid = factory.parseGuidFromString(inputUuid);

  // Convert the NumericGuid back to string format.
  std::string outputUuid = factory.parseGuid(testGuid);

  EXPECT_EQ(inputUuid, outputUuid);
}