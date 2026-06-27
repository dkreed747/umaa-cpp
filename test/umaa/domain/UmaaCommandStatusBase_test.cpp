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

#include "UmaaCommandStatusBase.h"

TEST(UmaaCommandStatusBaseTest, defaultConstructor) {
  arl::UmaaCommandStatusBase status;
  NumericGUID_t exp = {0};

  EXPECT_EQ(status.getSource(), exp);
  EXPECT_EQ(status.getSessionId(), exp);
}

TEST(UmaaCommandStatusBaseTest, setterConstructor) {
  NumericGUID_t source = {0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 114, 233, 0, 1};
  NumericGUID_t session = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
  arl::UmaaCommandStatusBase status(source, session);

  EXPECT_EQ(status.getSource(), source);
  EXPECT_EQ(status.getSessionId(), session);
}

TEST(UmaaCommandStatusBaseTest, sourceSetGet) {
  arl::UmaaCommandStatusBase status;
  NumericGUID_t exp = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

  status.setSource(exp);
  EXPECT_EQ(status.getSource(), exp);
}

TEST(UmaaCommandStatusBaseTest, sessionIdSetGet) {
  arl::UmaaCommandStatusBase status;
  NumericGUID_t exp = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

  status.setSessionId(exp);
  EXPECT_EQ(status.getSessionId(), exp);
}
