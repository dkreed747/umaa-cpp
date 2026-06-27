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

#include "UmaaCommandStatus.h"

TEST(UmaaCommandStatusTest, defaultConstructor) {
  arl::UmaaCommandStatus status;
  arl::CommandState exp_status = arl::CommandState::INVALID;
  arl::CommandStatus exp_reason = arl::CommandStatus::SERVICE_FAILED;
  std::string exp_message = "";

  EXPECT_EQ(status.getCommandStatus(), exp_status);
  EXPECT_EQ(status.getCommandStatusReason(), exp_reason);
  EXPECT_EQ(status.getLogMessage(), exp_message);
}

TEST(UmaaCommandStatusTest, setterConstructor) {
  NumericGUID_t guid = {0};
  arl::CommandState exp_status = arl::CommandState::ISSUED;
  arl::CommandStatus exp_reason = arl::CommandStatus::SUCCEEDED;
  std::string exp_message = "Command issued successfully";
  arl::UmaaCommandStatus status(guid, guid, exp_status, exp_reason, exp_message);

  EXPECT_EQ(status.getCommandStatus(), exp_status);
  EXPECT_EQ(status.getCommandStatusReason(), exp_reason);
  EXPECT_EQ(status.getLogMessage(), exp_message);
}

TEST(UmaaCommandStatusTest, commandStatusSetGet) {
  arl::UmaaCommandStatus status;
  arl::CommandState exp_status = arl::CommandState::ISSUED;

  status.setCommandStatus(exp_status);
  EXPECT_EQ(status.getCommandStatus(), exp_status);
}

TEST(UmaaCommandStatusTest, commandStatusReasonSetGet) {
  arl::UmaaCommandStatus status;
  arl::CommandStatus exp_reason = arl::CommandStatus::SUCCEEDED;

  status.setCommandStatusReason(exp_reason);
  EXPECT_EQ(status.getCommandStatusReason(), exp_reason);
}
