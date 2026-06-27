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

#include "UuidBase.h"

TEST(UuidBase_test, getId)
{
  arlcore::UuidBase baseUuid;
  arlcore::NumericGuid emptyGuid;

  // Verify the base UUID is not empty.
  EXPECT_FALSE(emptyGuid == baseUuid.getId());
}

TEST(UuidBase_test, getIdString)
{
  arlcore::UuidBase baseUuid;
  std::string uuidString = baseUuid.getIdString();

  std::cout << "Uuid: " << uuidString << std::endl;
}
