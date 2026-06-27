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

// See full GTest Documentation for reference:
// https://google.github.io/googletest/
#include <gtest/gtest.h>

#include "LocalReaderSender.h"
#include "MockObjective.h"
#include "ObjectiveExecutorStateControlServiceProvider.h"

class ObjectiveExecutorStateControlServiceProviderTest : public ::testing::Test {
 public:
  static void SetUpTestSuite() {
    // Create obj IO
    const auto objIo = std::make_shared<arlcore::umaa::ObjectiveExecutorStateControlServiceProviderIo>(
      objCmdLRS_ = std::make_shared<arlcore::io::LocalReaderSender<
        arlcore::umaa::ObjectiveExecutorStateCommandType>>(),
      objAckLRS_ = std::make_shared<arlcore::io::LocalReaderSender<
        arlcore::umaa::ObjectiveExecutorStateCommandAckReportType>>(),
      objCmdStatLRS_ = std::make_shared<arlcore::io::LocalReaderSender<
        arlcore::umaa::ObjectiveExecutorStateCommandStatusType>>(),
      objExeStatLRS_ = std::make_shared<arlcore::io::LocalReaderSender<
        arlcore::umaa::ObjectiveExecutorExecutionStatusReportType>>());

    // Create test numeric GUID id
    const NumericGuid testId({82, 111, 117, 116, 101, 79, 98, 106, 69, 120, 101, 99, 117, 116, 111, 114});

    // Create objective state control service provider test instance
    objExeStateCtrlProvider_ = std::make_unique<arlcore::umaa::ObjectiveExecutorStateControlServiceProvider>(
      testId,
      objIo,
      ObjectiveExecutorStateControlServiceProviderTest::getObjective);

    arlcore::umaa::ObjectiveType baseObj;

    baseObj.objectiveID() = testId.getGuid();

    objective_ = std::optional<std::shared_ptr<arlcore::umaa::ObjectiveBase>>(
      std::make_shared<arlcore::test::MockObjective>(baseObj, nullptr));

    // Test Command
    ObjectiveCmd_.destination().id() = arlcore::UuidFactory::getInstance().generateGuid().getGuid();
  }

  static std::optional<std::shared_ptr<arlcore::umaa::ObjectiveBase>> getObjective() {
    return testGetObjFail_ ?
      std::nullopt :
      objective_;
  }

 protected:
  static std::shared_ptr<arlcore::io::LocalReaderSender<arlcore::umaa::ObjectiveExecutorStateCommandType>>
    objCmdLRS_;
  static std::shared_ptr<arlcore::io::LocalReaderSender<arlcore::umaa::ObjectiveExecutorStateCommandAckReportType>>
    objAckLRS_;
  static std::shared_ptr<arlcore::io::LocalReaderSender<arlcore::umaa::ObjectiveExecutorStateCommandStatusType>>
    objCmdStatLRS_;
  static std::shared_ptr<arlcore::io::LocalReaderSender<arlcore::umaa::ObjectiveExecutorExecutionStatusReportType>>
    objExeStatLRS_;

  static std::unique_ptr<arlcore::umaa::ObjectiveExecutorStateControlServiceProvider> objExeStateCtrlProvider_;
  static arlcore::umaa::ObjectiveExecutorStateCommandType ObjectiveCmd_;
  static std::optional<std::shared_ptr<arlcore::umaa::ObjectiveBase>> objective_;

  static bool testGetObjFail_;
};

std::shared_ptr<arlcore::io::LocalReaderSender<arlcore::umaa::ObjectiveExecutorStateCommandType>>
  ObjectiveExecutorStateControlServiceProviderTest::objCmdLRS_;
std::shared_ptr<arlcore::io::LocalReaderSender<arlcore::umaa::ObjectiveExecutorStateCommandAckReportType>>
  ObjectiveExecutorStateControlServiceProviderTest::objAckLRS_;
std::shared_ptr<arlcore::io::LocalReaderSender<arlcore::umaa::ObjectiveExecutorStateCommandStatusType>>
  ObjectiveExecutorStateControlServiceProviderTest::objCmdStatLRS_;
std::shared_ptr<arlcore::io::LocalReaderSender<arlcore::umaa::ObjectiveExecutorExecutionStatusReportType>>
  ObjectiveExecutorStateControlServiceProviderTest::objExeStatLRS_;


std::unique_ptr<arlcore::umaa::ObjectiveExecutorStateControlServiceProvider>
  ObjectiveExecutorStateControlServiceProviderTest::objExeStateCtrlProvider_;
arlcore::umaa::ObjectiveExecutorStateCommandType
  ObjectiveExecutorStateControlServiceProviderTest::ObjectiveCmd_;
std::optional<std::shared_ptr<arlcore::umaa::ObjectiveBase>>
  ObjectiveExecutorStateControlServiceProviderTest::objective_ = std::nullopt;

bool ObjectiveExecutorStateControlServiceProviderTest::testGetObjFail_ = true;

TEST_F(ObjectiveExecutorStateControlServiceProviderTest, testStateChanges) {
  // Dry cycle run. Should not have active command or status
  ASSERT_TRUE(objExeStateCtrlProvider_->cycle());
  EXPECT_FALSE(objExeStateCtrlProvider_->getActiveCommand());
  EXPECT_FALSE(objExeStateCtrlProvider_->getCommandStatus());

  ObjectiveCmd_.missionID() = arlcore::UuidFactory::getInstance().generateGuid().getGuid();
  ObjectiveCmd_.taskID() = arlcore::UuidFactory::getInstance().generateGuid().getGuid();
  ObjectiveCmd_.objectiveID() = arlcore::UuidFactory::getInstance().generateGuid().getGuid();
  ObjectiveCmd_.source().id() = arlcore::UuidFactory::getInstance().generateGuid().getGuid();
  ObjectiveCmd_.sessionID() = arlcore::UuidFactory::getInstance().generateGuid().getGuid();
  objCmdLRS_->send(ObjectiveCmd_);

  ASSERT_TRUE(objExeStateCtrlProvider_->cycle());

  UMAA::MM::ObjectiveExecutorControl::ObjectiveExecutorStateCommandStatusType status;
  ASSERT_EQ(objCmdStatLRS_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::ISSUED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  ASSERT_EQ(objCmdStatLRS_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::FAILED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::VALIDATION_FAILED);
  ASSERT_EQ(objCmdStatLRS_->read(&status), ReadStatus::DISPOSED);

  // Another cycle will move the provider state back to IDLE
  ASSERT_TRUE(objExeStateCtrlProvider_->cycle());
  EXPECT_FALSE(objExeStateCtrlProvider_->getActiveCommand());
  EXPECT_FALSE(objExeStateCtrlProvider_->getCommandStatus());

  // Send an objective that does not match
  testGetObjFail_ = false;
  objCmdLRS_->send(ObjectiveCmd_);

  // First cycle with new command where validation will fail
  ASSERT_TRUE(objExeStateCtrlProvider_->cycle());

  ASSERT_EQ(objCmdStatLRS_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::ISSUED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  ASSERT_EQ(objCmdStatLRS_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::FAILED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::VALIDATION_FAILED);
  ASSERT_EQ(objCmdStatLRS_->read(&status), ReadStatus::DISPOSED);

  // Another cycle will move the provider state back to IDLE
  ASSERT_TRUE(objExeStateCtrlProvider_->cycle());
  EXPECT_FALSE(objExeStateCtrlProvider_->getActiveCommand());
  EXPECT_FALSE(objExeStateCtrlProvider_->getCommandStatus());

  // Send an objective that does match
  ObjectiveCmd_.objectiveID() = objective_.value()->getId().getGuid();
  objCmdLRS_->send(ObjectiveCmd_);

  // First cycle with new command where validation will pass. Will move state to ISSUED
  ASSERT_TRUE(objExeStateCtrlProvider_->cycle());

  ASSERT_EQ(objCmdStatLRS_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::ISSUED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  ASSERT_EQ(objCmdStatLRS_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMMANDED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  ASSERT_EQ(objCmdStatLRS_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::EXECUTING);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  ASSERT_EQ(objCmdStatLRS_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMPLETED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  ASSERT_EQ(objCmdStatLRS_->read(&status), ReadStatus::DISPOSED);

  // Sixth cycle will return provider to IDLE state
  ASSERT_TRUE(objExeStateCtrlProvider_->cycle());
  EXPECT_FALSE(objExeStateCtrlProvider_->getActiveCommand());
}
