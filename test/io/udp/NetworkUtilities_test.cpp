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

#include "NetworkUtilities.h"

//! \brief Perform a test against localhost for IP resolution.
TEST(NetworkUtilitiesTest, getIpAddressFromHostName) {
  std::string retIp = arlcore::io::NetworkUtilities::getIpAddressFromHostName("localhost");
  std::string expIp = "127.0.0.1";

  EXPECT_EQ(expIp, retIp);
}

//! \brief Perform a test against a what should be fake URL and expect
//!  an emptry string returned.
TEST(NetworkUtilititesTest, getIpAddressFromHostNameFail) {
  std::string retIp = arlcore::io::NetworkUtilities::getIpAddressFromHostName("testhostname");

  // expect that no IP is resolved and an empty string is returned.
  EXPECT_TRUE(retIp.empty());
}
