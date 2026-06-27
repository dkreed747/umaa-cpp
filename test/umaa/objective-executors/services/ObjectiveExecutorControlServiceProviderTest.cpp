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

#include "MockObjectiveFactory.h"
#include "ObjectiveExecutorControlServiceProvider.h"
#include "ObjectiveFactory.h"
#include "LocalReaderSender.h"
#include "LargeListWriter.h"
#include "MockConditional.h"

class ObjectiveExecutorControlServiceProviderTest : public ::testing::Test {
 protected:
  static void SetUpTestSuite() {
    // Create obj IO
    const auto objIo = std::make_shared<arlcore::umaa::ObjectiveExecutorControlServiceProviderIo>(
      objCmdLRS_ = std::make_shared<arlcore::io::LocalReaderSender<
        arlcore::umaa::ObjectiveExecutorCommandType>>(),
      objAckLRS_ = std::make_shared<arlcore::io::LocalReaderSender<
        arlcore::umaa::ObjectiveExecutorCommandAckReportType>>(),
      objCmdStatLRS_ = std::make_shared<arlcore::io::LocalReaderSender<
        arlcore::umaa::ObjectiveExecutorCommandStatusType>>(),
      objExeStatLRS_ = std::make_shared<arlcore::io::LocalReaderSender<
        arlcore::umaa::ObjectiveExecutorExecutionStatusReportType>>());

    // Create test mock objective factory
    mockObjFactory_ = std::make_shared<arlcore::test::MockObjectiveFactory>();

    // Create test numeric GUID id
    const NumericGuid testId({82, 111, 117, 116, 101, 79, 98, 106, 69, 120, 101, 99, 117, 116, 111, 114});

    // Create objective control service provider test instance
    objExeCtrlSvcProvider_ = std::make_unique<arlcore::umaa::ObjectiveExecutorControlServiceProvider>(
      testId,
      objIo,
      mockObjFactory_);

    // Test Command
    ObjectiveCmd_.destination().id(testId.getGuid());
  }

  static void TearDownTestSuite() {
    // Code to run at the end of the test suite
  }

  void SetUp() override {
    // Code to run before running each TEST_F()
  }

  void TearDown() override {
    // Code to run after running each TEST_F()
    objExeCtrlSvcProvider_->setActiveConstraintsProvider(std::nullopt);
    mockObjFactory_->testBuildFail = true;
    mockObjFactory_->testObjectiveValidationFail = false;
    objCmdLRS_->clear();
    objAckLRS_->clear();
    objCmdStatLRS_->clear();
    objExeStatLRS_->clear();
  }

  // Static Declarations

  // Local Obj. IO
  static std::shared_ptr<arlcore::io::LocalReaderSender<arlcore::umaa::ObjectiveExecutorCommandType>>
    objCmdLRS_;
  static std::shared_ptr<arlcore::io::LocalReaderSender<arlcore::umaa::ObjectiveExecutorCommandAckReportType>>
    objAckLRS_;
  static std::shared_ptr<arlcore::io::LocalReaderSender<arlcore::umaa::ObjectiveExecutorCommandStatusType>>
    objCmdStatLRS_;
  static std::shared_ptr<arlcore::io::LocalReaderSender<arlcore::umaa::ObjectiveExecutorExecutionStatusReportType>>
    objExeStatLRS_;

  // Test assets
  static arlcore::umaa::ObjectiveExecutorCommandType ObjectiveCmd_;
  static std::shared_ptr<arlcore::test::MockObjectiveFactory> mockObjFactory_;
  static std::unique_ptr<arlcore::umaa::ObjectiveExecutorControlServiceProvider> objExeCtrlSvcProvider_;
};

// Static Definitions

// Local Obj. IO
std::shared_ptr<arlcore::io::LocalReaderSender<arlcore::umaa::ObjectiveExecutorCommandType>>
 ObjectiveExecutorControlServiceProviderTest::objCmdLRS_;
std::shared_ptr<arlcore::io::LocalReaderSender<arlcore::umaa::ObjectiveExecutorCommandAckReportType>>
 ObjectiveExecutorControlServiceProviderTest::objAckLRS_;
std::shared_ptr<arlcore::io::LocalReaderSender<arlcore::umaa::ObjectiveExecutorCommandStatusType>>
 ObjectiveExecutorControlServiceProviderTest::objCmdStatLRS_;
std::shared_ptr<arlcore::io::LocalReaderSender<arlcore::umaa::ObjectiveExecutorExecutionStatusReportType>>
 ObjectiveExecutorControlServiceProviderTest::objExeStatLRS_;

arlcore::umaa::ObjectiveExecutorCommandType
  ObjectiveExecutorControlServiceProviderTest::ObjectiveCmd_;
std::shared_ptr<arlcore::test::MockObjectiveFactory>
  ObjectiveExecutorControlServiceProviderTest::mockObjFactory_;
std::unique_ptr<arlcore::umaa::ObjectiveExecutorControlServiceProvider>
  ObjectiveExecutorControlServiceProviderTest::objExeCtrlSvcProvider_;

TEST_F(ObjectiveExecutorControlServiceProviderTest, testRegisterObserver) {
  auto activeConstraintsCommandReader_ = std::make_shared<arlcore::io::LocalReaderSender<UMAA::MM::ActiveConstraintsControl::ActiveConstraintsCommandType>>();
  auto constraintsP_ = std::make_shared<arlcore::umaa::conditional::ActiveConstraintsControlProvider>(
    arlcore::UuidFactory::getInstance().generateGuid(),
    std::make_shared<ActiveConstraintsControlProviderIo>(
      activeConstraintsCommandReader_,
      std::make_shared<arlcore::io::LocalReaderSender<UMAA::MM::ActiveConstraintsControl::ActiveConstraintsCommandAckReportType>>(),
      std::make_shared<arlcore::io::LocalReaderSender<UMAA::MM::ActiveConstraintsControl::ActiveConstraintsCommandStatusType>>()
    )
  );

  std::vector<std::shared_ptr<arlcore::umaa::conditional::ConditionalBase>> conditionals;
  UMAA::MM::Conditional::ConditionalType constraintConditional;
  constraintConditional.conditionalID(arlcore::UuidFactory::getInstance().generateGuid().getGuid());
  constraintConditional.name("True constraint conditional");
  std::shared_ptr<arlcore::umaa::conditional::ConditionalBase> conditionalPtr = std::make_shared<arlcore::test::MockConditional>(constraintConditional, true);
  conditionals.push_back(conditionalPtr);

  constraintsP_->update(conditionals);

  UMAA::MM::ActiveConstraintsControl::ActiveConstraintsCommandType activeConstraintsCommand;
  activeConstraintsCommand.constraintConditionalIDs().push_back(constraintConditional.conditionalID());
  activeConstraintsCommandReader_->send(activeConstraintsCommand);

  // Cycle to process command
  constraintsP_->cycle();

  objExeCtrlSvcProvider_->setActiveConstraintsProvider(constraintsP_);

  // Set MockFactory to build a real objective
  mockObjFactory_->testBuildFail = false;

  // Send another objective command
  objCmdLRS_->send(ObjectiveCmd_);

  // Execute another cycle and check command is received and issued
  ASSERT_TRUE(objExeCtrlSvcProvider_->cycle());
  ASSERT_TRUE(objExeCtrlSvcProvider_->getActiveCommand());
  ASSERT_TRUE(objExeCtrlSvcProvider_->getCommandStatus());
  ASSERT_TRUE(objExeCtrlSvcProvider_->getCommandStatusReason());
  EXPECT_EQ(objExeCtrlSvcProvider_->getCommandStatus().value(), CommandStatusEnumType::EXECUTING);
  EXPECT_EQ(objExeCtrlSvcProvider_->getCommandStatusReason().value(), CommandStatusReasonEnumType::SUCCEEDED);

  auto optObj = objExeCtrlSvcProvider_->getActiveObjective();
  ASSERT_TRUE(optObj.has_value());
  EXPECT_EQ(optObj.value()->getObjectiveState(),
    arlcore::umaa::ObjectiveExecutorStateEnumType::QUEUED);
  auto obj = std::dynamic_pointer_cast<arlcore::test::MockObjective>(optObj.value());
  auto constraintsOpt = obj->getActiveConstraints();
  ASSERT_TRUE(constraintsP_->getConstraintConditionals().has_value());
  ASSERT_TRUE(constraintsOpt.has_value());
  EXPECT_EQ(constraintsOpt.value(), constraintsP_->getConstraintConditionals().value());

  // EXECUTING Command
  ASSERT_TRUE(objExeCtrlSvcProvider_->cycle());

  // QUEUED --> EXECUTING
  ASSERT_TRUE(objExeCtrlSvcProvider_->getActiveObjective());
  auto mockObj = objExeCtrlSvcProvider_->getActiveObjective().value();
  ASSERT_TRUE(mockObj->advanceObjectiveState());
  EXPECT_EQ(mockObj->getObjectiveState(), arlcore::umaa::ObjectiveExecutorStateEnumType::EXECUTING);

  // EXECUTING --> COMPLETED
  ASSERT_TRUE(mockObj->advanceObjectiveState());
  EXPECT_EQ(mockObj->getObjectiveState(), arlcore::umaa::ObjectiveExecutorStateEnumType::COMPLETED);

  // COMPLETED Command
  ASSERT_TRUE(objExeCtrlSvcProvider_->cycle());
}

TEST_F(ObjectiveExecutorControlServiceProviderTest, testCommandFlow) {
  // Execute one cycle and check default values
  ASSERT_TRUE(objExeCtrlSvcProvider_->cycle());
  EXPECT_FALSE(objExeCtrlSvcProvider_->getActiveCommand());
  EXPECT_FALSE(objExeCtrlSvcProvider_->getCommandStatus());
  EXPECT_FALSE(objExeCtrlSvcProvider_->getCommandStatus());
  EXPECT_FALSE(objExeCtrlSvcProvider_->getActiveObjective());
  EXPECT_EQ(objExeCtrlSvcProvider_->getBehavior(), arlcore::umaa::services::IncomingCommandBehavior::REJECT_INCOMING);

  // Send an objective command
  objCmdLRS_->send(ObjectiveCmd_);

  // Execute another cycle and check command is received and issued
  ASSERT_TRUE(objExeCtrlSvcProvider_->cycle());
  EXPECT_FALSE(objExeCtrlSvcProvider_->getActiveCommand());

  UMAA::MM::ObjectiveExecutorControl::ObjectiveExecutorCommandStatusType status;
  ASSERT_EQ(objCmdStatLRS_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::ISSUED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  ASSERT_EQ(objCmdStatLRS_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::FAILED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::VALIDATION_FAILED);

  // Execute another cycle to see the provider go back to IDLE
  ASSERT_TRUE(objExeCtrlSvcProvider_->cycle());
  EXPECT_FALSE(objExeCtrlSvcProvider_->getActiveCommand());
  EXPECT_FALSE(objExeCtrlSvcProvider_->getCommandStatus());
  EXPECT_FALSE(objExeCtrlSvcProvider_->getCommandStatus());
  EXPECT_FALSE(objExeCtrlSvcProvider_->getActiveObjective());

  // Set MockFactory to build a real objective
  mockObjFactory_->testBuildFail = false;

  // Send another objective command
  objCmdLRS_->send(ObjectiveCmd_);

  // Execute another cycle and check command is received and issued
  ASSERT_TRUE(objExeCtrlSvcProvider_->cycle());
  ASSERT_TRUE(objExeCtrlSvcProvider_->getActiveCommand());
  ASSERT_TRUE(objExeCtrlSvcProvider_->getCommandStatus());
  ASSERT_TRUE(objExeCtrlSvcProvider_->getCommandStatus());

  EXPECT_EQ(objExeCtrlSvcProvider_->getCommandStatus().value(), CommandStatusEnumType::EXECUTING);
  EXPECT_EQ(objExeCtrlSvcProvider_->getCommandStatusReason().value(), CommandStatusReasonEnumType::SUCCEEDED);

  // QUEUED --> EXECUTING
  ASSERT_TRUE(objExeCtrlSvcProvider_->getActiveObjective());
  auto mockObj = objExeCtrlSvcProvider_->getActiveObjective().value();
  ASSERT_TRUE(mockObj->advanceObjectiveState());
  EXPECT_EQ(mockObj->getObjectiveState(), arlcore::umaa::ObjectiveExecutorStateEnumType::EXECUTING);

  // EXECUTING --> COMPLETED
  ASSERT_TRUE(mockObj->advanceObjectiveState());
  EXPECT_EQ(mockObj->getObjectiveState(), arlcore::umaa::ObjectiveExecutorStateEnumType::COMPLETED);

  objCmdStatLRS_->clear();

  // Execute another cycle and check the command has been completed since the objective is now complete
  ASSERT_TRUE(objExeCtrlSvcProvider_->cycle());
  ASSERT_EQ(objCmdStatLRS_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMPLETED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  ASSERT_EQ(objCmdStatLRS_->read(&status), ReadStatus::DISPOSED);

  ASSERT_TRUE(objExeCtrlSvcProvider_->cycle());
  ASSERT_FALSE(objExeCtrlSvcProvider_->getActiveCommand());
  ASSERT_FALSE(objExeCtrlSvcProvider_->getCommandStatus());
  ASSERT_FALSE(objExeCtrlSvcProvider_->getCommandStatus());
  ASSERT_FALSE(objExeCtrlSvcProvider_->getActiveObjective());
}

TEST_F(ObjectiveExecutorControlServiceProviderTest, objectiveValidation) {
  // Execute one cycle and check default values
  ASSERT_TRUE(objExeCtrlSvcProvider_->cycle());
  EXPECT_FALSE(objExeCtrlSvcProvider_->getActiveCommand());
  EXPECT_FALSE(objExeCtrlSvcProvider_->getCommandStatus());
  EXPECT_FALSE(objExeCtrlSvcProvider_->getCommandStatus());
  EXPECT_FALSE(objExeCtrlSvcProvider_->getActiveObjective());
  EXPECT_EQ(objExeCtrlSvcProvider_->getBehavior(), arlcore::umaa::services::IncomingCommandBehavior::REJECT_INCOMING);

  // Set MockFactory to build a real objective but set the objective validation to fail
  mockObjFactory_->testBuildFail = false;
  mockObjFactory_->testObjectiveValidationFail = true;

  // Send an objective command
  objCmdLRS_->send(ObjectiveCmd_);

  // Execute another cycle and check command is received and issued
  ASSERT_TRUE(objExeCtrlSvcProvider_->cycle());

  UMAA::MM::ObjectiveExecutorControl::ObjectiveExecutorCommandStatusType status;
  ASSERT_EQ(objCmdStatLRS_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::ISSUED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  ASSERT_EQ(objCmdStatLRS_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::FAILED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::VALIDATION_FAILED);

  // Execute another cycle to see the provider go back to IDLE
  ASSERT_TRUE(objExeCtrlSvcProvider_->cycle());
  EXPECT_FALSE(objExeCtrlSvcProvider_->getActiveCommand());
  EXPECT_FALSE(objExeCtrlSvcProvider_->getCommandStatus());
  EXPECT_FALSE(objExeCtrlSvcProvider_->getCommandStatus());
  EXPECT_FALSE(objExeCtrlSvcProvider_->getActiveObjective());
}
