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
#include <csignal>
#include <thread>

#include "LocalReaderSender.h"
#include "MockConditional.h"
#include "MockObjective.h"
#include "MockObjectiveFactory.h"
#include "ObjectiveController.h"
#include "ConditionalReportConsumer.h"
#include "LocalConditionalFactoryIo.h"

class ObjectiveControllerTest: public testing::Test {
 protected:
  ObjectiveControllerTest() {}

  static void SetUpTestSuite() {
    objFactory_ = std::make_shared<arlcore::test::MockObjectiveFactory>();
    objFactory_->testBuildFail = false;  // Alow builder to successfully create mock objective

    constraintsP_ = std::make_shared<arlcore::umaa::conditional::ActiveConstraintsControlProvider>(
      sourceId_,
      std::make_shared<ActiveConstraintsControlProviderIo>(
        activeConstraintsCommandReader_ = std::make_shared<arlcore::io::LocalReaderSender<UMAA::MM::ActiveConstraintsControl::ActiveConstraintsCommandType>>(),
        activeConstraintsAckSender_ = std::make_shared<arlcore::io::LocalReaderSender<UMAA::MM::ActiveConstraintsControl::ActiveConstraintsCommandAckReportType>>(),
        activeConstraintsStatusSender_ = std::make_shared<arlcore::io::LocalReaderSender<UMAA::MM::ActiveConstraintsControl::ActiveConstraintsCommandStatusType>>()
      )
    );

    localConditionalFactoryIo_ = std::make_shared<arlcore::test::LocalConditionalFactoryIo>();

    condReportC_ = std::make_shared<arlcore::umaa::conditional::ConditionalReportConsumer>(
      conditionalReportReader_ = std::make_shared<arlcore::io::LocalReaderSender<UMAA::MM::ConditionalReport::ConditionalReportType>>(),
      reportSetElementReader_ = std::make_shared<arlcore::io::LocalReaderSender<UMAA::MM::ConditionalReport::ConditionalReportTypeConditionalsSetElement>>(),
      std::make_shared<arlcore::umaa::conditional::ConditionalFactory>(localConditionalFactoryIo_->io_)
    );

    objExeCtrlSvcP_ = std::make_shared<arlcore::umaa::ObjectiveExecutorControlServiceProvider>(
      sourceId_,
      std::make_shared<arlcore::umaa::ObjectiveExecutorControlServiceProviderIo>(
        objCmdReader_ = std::make_shared<arlcore::io::LocalReaderSender<
          arlcore::umaa::ObjectiveExecutorCommandType>>(),
        objAckSender_ = std::make_shared<arlcore::io::LocalReaderSender<
          arlcore::umaa::ObjectiveExecutorCommandAckReportType>>(),
        objStatSender_ = std::make_shared<arlcore::io::LocalReaderSender<
          arlcore::umaa::ObjectiveExecutorCommandStatusType>>(),
        objExeStatSender_ = std::make_shared<arlcore::io::LocalReaderSender<
          arlcore::umaa::ObjectiveExecutorExecutionStatusReportType>>()),
      objFactory_);

    objExeStateCtrlSvcP_ = std::make_shared<arlcore::umaa::ObjectiveExecutorStateControlServiceProvider>(
      sourceId_,
      std::make_shared<arlcore::umaa::ObjectiveExecutorStateControlServiceProviderIo>(
        objStateCmdReader_ = std::make_shared<arlcore::io::LocalReaderSender<
          arlcore::umaa::ObjectiveExecutorStateCommandType>>(),
        objStateAckSender_ = std::make_shared<arlcore::io::LocalReaderSender<
          arlcore::umaa::ObjectiveExecutorStateCommandAckReportType>>(),
        objStateStatSender_ = std::make_shared<arlcore::io::LocalReaderSender<
          arlcore::umaa::ObjectiveExecutorStateCommandStatusType>>(),
        objExeStatSender_),
      objExeCtrlSvcP_->getActiveObjective);

    objCtrl_ = std::make_unique<arlcore::umaa::ObjectiveController>(
      std::string(registrationTopic_),
      localConditionalFactoryIo_->io_->globalPoseReportConsumer_,
      localConditionalFactoryIo_->io_->speedReportConsumer_,
      localConditionalFactoryIo_->io_->velocityReportConsumer_,
      objExeCtrlSvcP_,
      objExeStateCtrlSvcP_,
      constraintsP_,
      condReportC_);

    std::signal(SIGINT, objCtrl_->signalHandler);
    std::signal(SIGTERM, objCtrl_->signalHandler);
    std::signal(SIGILL, objCtrl_->signalHandler);
  }

  static void TearDownTestSuite() {}

  virtual void SetUp() {}

  virtual void TearDown() {
    constraintsP_->update({});
    UMAA::MM::ActiveConstraintsControl::ActiveConstraintsCommandType activeConstraintsCommand;
    activeConstraintsCommand.constraintConditionalIDs().clear();
    activeConstraintsCommandReader_->send(activeConstraintsCommand);
    localConditionalFactoryIo_->clearAll();
  }

  // Helper function to raise a signal
  static void raiseSignal(int32_t inSignal) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::raise(inSignal);
  }

  // Test Assets
  static const arlcore::NumericGuid sourceId_;
  static const char* registrationTopic_;

  static std::shared_ptr<arlcore::test::MockObjectiveFactory> objFactory_;

  static std::unique_ptr<arlcore::umaa::ObjectiveController> objCtrl_;

  // Conditionals and Constraints
  static std::shared_ptr<arlcore::test::LocalConditionalFactoryIo> localConditionalFactoryIo_;
  static std::shared_ptr<arlcore::io::LocalReaderSender<UMAA::MM::ConditionalReport::ConditionalReportType>> conditionalReportReader_;
  static std::shared_ptr<arlcore::io::LocalReaderSender<UMAA::MM::ConditionalReport::ConditionalReportTypeConditionalsSetElement>> reportSetElementReader_;
  static std::shared_ptr<arlcore::io::LocalReaderSender<UMAA::MM::ActiveConstraintsControl::ActiveConstraintsCommandType>> activeConstraintsCommandReader_;
  static std::shared_ptr<arlcore::io::LocalReaderSender<UMAA::MM::ActiveConstraintsControl::ActiveConstraintsCommandAckReportType>> activeConstraintsAckSender_;
  static std::shared_ptr<arlcore::io::LocalReaderSender<UMAA::MM::ActiveConstraintsControl::ActiveConstraintsCommandStatusType>> activeConstraintsStatusSender_;
  static std::shared_ptr<arlcore::umaa::conditional::ActiveConstraintsControlProvider> constraintsP_;
  static std::shared_ptr<arlcore::umaa::conditional::ConditionalReportConsumer> condReportC_;

  // Objective Executor Control Service
  static std::shared_ptr<arlcore::io::LocalReaderSender<arlcore::umaa::ObjectiveExecutorCommandType>>
    objCmdReader_;
  static std::shared_ptr<arlcore::io::LocalReaderSender<arlcore::umaa::ObjectiveExecutorCommandAckReportType>>
    objAckSender_;
  static std::shared_ptr<arlcore::io::LocalReaderSender<arlcore::umaa::ObjectiveExecutorCommandStatusType>>
    objStatSender_;
  static std::shared_ptr<arlcore::io::LocalReaderSender<arlcore::umaa::ObjectiveExecutorExecutionStatusReportType>>
    objExeStatSender_;
  static std::shared_ptr<arlcore::umaa::ObjectiveExecutorControlServiceProvider>
    objExeCtrlSvcP_;

  // Objective Executor State Control Service
  static std::shared_ptr<arlcore::io::LocalReaderSender<arlcore::umaa::ObjectiveExecutorStateCommandType>>
    objStateCmdReader_;
  static std::shared_ptr<arlcore::io::LocalReaderSender<arlcore::umaa::ObjectiveExecutorStateCommandAckReportType>>
    objStateAckSender_;
  static std::shared_ptr<arlcore::io::LocalReaderSender<arlcore::umaa::ObjectiveExecutorStateCommandStatusType>>
    objStateStatSender_;
  static std::shared_ptr<arlcore::umaa::ObjectiveExecutorStateControlServiceProvider>
    objExeStateCtrlSvcP_;
};

// Test Assets
const arlcore::NumericGuid ObjectiveControllerTest::sourceId_ = arlcore::UuidFactory::getInstance().generateGuid();
const char* ObjectiveControllerTest::registrationTopic_ = "UMAA::TestTypeTopic";

std::shared_ptr<arlcore::test::MockObjectiveFactory>
  ObjectiveControllerTest::objFactory_;

std::unique_ptr<arlcore::umaa::ObjectiveController> ObjectiveControllerTest::objCtrl_;

// Conditionals and Constraints
std::shared_ptr<arlcore::test::LocalConditionalFactoryIo> ObjectiveControllerTest::localConditionalFactoryIo_;
std::shared_ptr<arlcore::io::LocalReaderSender<UMAA::MM::ConditionalReport::ConditionalReportType>> ObjectiveControllerTest::conditionalReportReader_;
std::shared_ptr<arlcore::io::LocalReaderSender<UMAA::MM::ConditionalReport::ConditionalReportTypeConditionalsSetElement>> ObjectiveControllerTest::reportSetElementReader_;
std::shared_ptr<arlcore::io::LocalReaderSender<UMAA::MM::ActiveConstraintsControl::ActiveConstraintsCommandType>> ObjectiveControllerTest::activeConstraintsCommandReader_;
std::shared_ptr<arlcore::io::LocalReaderSender<UMAA::MM::ActiveConstraintsControl::ActiveConstraintsCommandAckReportType>> ObjectiveControllerTest::activeConstraintsAckSender_;
std::shared_ptr<arlcore::io::LocalReaderSender<UMAA::MM::ActiveConstraintsControl::ActiveConstraintsCommandStatusType>> ObjectiveControllerTest::activeConstraintsStatusSender_;
std::shared_ptr<arlcore::umaa::conditional::ActiveConstraintsControlProvider> ObjectiveControllerTest::constraintsP_;
std::shared_ptr<arlcore::umaa::conditional::ConditionalReportConsumer> ObjectiveControllerTest::condReportC_;

// Objective Executor Control Service
std::shared_ptr<arlcore::io::LocalReaderSender<arlcore::umaa::ObjectiveExecutorCommandType>>
  ObjectiveControllerTest::objCmdReader_;
std::shared_ptr<arlcore::io::LocalReaderSender<arlcore::umaa::ObjectiveExecutorCommandAckReportType>>
  ObjectiveControllerTest::objAckSender_;
std::shared_ptr<arlcore::io::LocalReaderSender<arlcore::umaa::ObjectiveExecutorCommandStatusType>>
  ObjectiveControllerTest::objStatSender_;
std::shared_ptr<arlcore::io::LocalReaderSender<arlcore::umaa::ObjectiveExecutorExecutionStatusReportType>>
  ObjectiveControllerTest::objExeStatSender_;
std::shared_ptr<arlcore::umaa::ObjectiveExecutorControlServiceProvider>
  ObjectiveControllerTest::objExeCtrlSvcP_;

// Objective Executor State Control Service
std::shared_ptr<arlcore::io::LocalReaderSender<arlcore::umaa::ObjectiveExecutorStateCommandType>>
  ObjectiveControllerTest::objStateCmdReader_;
std::shared_ptr<arlcore::io::LocalReaderSender<arlcore::umaa::ObjectiveExecutorStateCommandAckReportType>>
  ObjectiveControllerTest::objStateAckSender_;
std::shared_ptr<arlcore::io::LocalReaderSender<arlcore::umaa::ObjectiveExecutorStateCommandStatusType>>
  ObjectiveControllerTest::objStateStatSender_;
std::shared_ptr<arlcore::umaa::ObjectiveExecutorStateControlServiceProvider>
  ObjectiveControllerTest::objExeStateCtrlSvcP_;

TEST_F(ObjectiveControllerTest, signalHandlerTest) {
  // This test runs the ObjectiveController and terminates with SIGTERM, SIGINT, and an unknown signal
  // 1. Trigger SIGTERM Signal
  // 2. Trigger SIGINT Signal
  // 3. Trigger Unknown Signal

  std::thread sigTermThread(raiseSignal, SIGTERM);
  ASSERT_EQ(objCtrl_->run(), 0);
  sigTermThread.join();

  std::thread sigIntThread(raiseSignal, SIGINT);
  ASSERT_EQ(objCtrl_->run(), 0);
  sigIntThread.join();

  std::thread sigUnknownThread(raiseSignal, SIGILL);
  ASSERT_EQ(objCtrl_->run(), 0);
  sigUnknownThread.join();
}

TEST_F(ObjectiveControllerTest, NominalObjectiveExecution) {
  // This test will send an objective to the Objective Executor and walk it through nominal execution

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

  arlcore::umaa::ObjectiveType obj;
  obj.name() = "Generalized Objective 1";
  obj.objectiveID() = arlcore::UuidFactory::getInstance().generateGuid().getGuid();

  arlcore::umaa::ObjectiveExecutorCommandType objCmd;
  objCmd.objective() = obj;

  // Cycle dry-run - Providers should be IDLE and no active objective
  ASSERT_TRUE(objCtrl_->cycle());
  ASSERT_FALSE(objExeCtrlSvcP_->getCommandStatus());
  ASSERT_FALSE(objExeStateCtrlSvcP_->getCommandStatus());
  ASSERT_FALSE(objExeCtrlSvcP_->getActiveObjective());

  ASSERT_EQ(objCmdReader_->send(objCmd), SendStatus::SUCCESS);

  // Cycle and expect ObjectiveExecutorControlProvider to go to EXECUTING
  ASSERT_TRUE(objCtrl_->cycle());
  ASSERT_TRUE(objExeCtrlSvcP_->getCommandStatus());
  EXPECT_EQ(objExeCtrlSvcP_->getCommandStatus().value(), CommandStatusEnumType::EXECUTING);
  ASSERT_FALSE(objExeStateCtrlSvcP_->getCommandStatus());
  EXPECT_TRUE(objExeCtrlSvcP_->getActiveObjective());

  EXPECT_EQ(objExeCtrlSvcP_->getActiveObjective().value()->getName(), obj.name());
  EXPECT_EQ(objExeCtrlSvcP_->getActiveObjective().value()->getObjectiveState(),
    arlcore::umaa::ObjectiveExecutorStateEnumType::QUEUED);

  // Cycle and expect ObjectiveExecutorControlProvider to still be EXECUTING and objective to still be QUEUED.
  // The objective ctrl command will not complete until the objective is complete
  ASSERT_TRUE(objCtrl_->cycle());
  ASSERT_TRUE(objExeCtrlSvcP_->getCommandStatus());
  ASSERT_FALSE(objExeStateCtrlSvcP_->getCommandStatus());
  ASSERT_TRUE(objExeCtrlSvcP_->getActiveObjective());
  EXPECT_EQ(objExeCtrlSvcP_->getCommandStatus().value(), CommandStatusEnumType::EXECUTING);
  EXPECT_EQ(objExeCtrlSvcP_->getActiveObjective().value()->getObjectiveState(),
    arlcore::umaa::ObjectiveExecutorStateEnumType::QUEUED);

  // Send an EXECUTE state command
  arlcore::umaa::ObjectiveExecutorStateCommandType objStateCmd;
  objStateCmd.objectiveID() = obj.objectiveID();
  objStateCmd.destination() = arlcore::umaa::IdentifierType(sourceId_.getGuid(), {});
  objStateCmd.objectiveState() = arlcore::umaa::ObjectiveExecutorControlEnumType::EXECUTE;
  objStateCmdReader_->send(objStateCmd);

  // Cycle and expect ObjectiveExecutorControlProvider to still be EXECUTING and objective to still be QUEUED.
  // The ObjectiveExecutorStateControlProvider will be ISSUED
  ASSERT_TRUE(objCtrl_->cycle());
  ASSERT_TRUE(objExeCtrlSvcP_->getCommandStatus());
  ASSERT_TRUE(objExeCtrlSvcP_->getActiveObjective());
  EXPECT_EQ(objExeCtrlSvcP_->getCommandStatus().value(), CommandStatusEnumType::EXECUTING);
  EXPECT_EQ(objExeCtrlSvcP_->getActiveObjective().value()->getObjectiveState(),
    arlcore::umaa::ObjectiveExecutorStateEnumType::COMPLETED);

  UMAA::MM::ObjectiveExecutorControl::ObjectiveExecutorStateCommandStatusType status;
  ASSERT_EQ(objStateStatSender_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::ISSUED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  ASSERT_EQ(objStateStatSender_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMMANDED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  ASSERT_EQ(objStateStatSender_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::EXECUTING);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  ASSERT_EQ(objStateStatSender_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMPLETED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  ASSERT_EQ(objStateStatSender_->read(&status), ReadStatus::DISPOSED);

  // Cycle and expect ObjectiveExecutorControlProvider to be COMPLETED and objective to still be COMPLETED.
  // The ObjectiveExecutorStateControlProvider will be COMPLETED
  objStatSender_->clear();
  ASSERT_TRUE(objCtrl_->cycle());
  EXPECT_FALSE(objExeCtrlSvcP_->getCommandStatus());
  EXPECT_FALSE(objExeStateCtrlSvcP_->getCommandStatus());
  ASSERT_TRUE(objExeCtrlSvcP_->getActiveObjective());
  EXPECT_EQ(objExeCtrlSvcP_->getActiveObjective().value()->getObjectiveState(),
    arlcore::umaa::ObjectiveExecutorStateEnumType::COMPLETED);

  UMAA::MM::ObjectiveExecutorControl::ObjectiveExecutorCommandStatusType cmdStatus;
  ASSERT_EQ(objStatSender_->read(&cmdStatus), ReadStatus::SUCCESS);
  EXPECT_EQ(cmdStatus.commandStatus(), CommandStatusEnumType::COMPLETED);
  EXPECT_EQ(cmdStatus.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  ASSERT_EQ(objStatSender_->read(&cmdStatus), ReadStatus::DISPOSED);

  // Cycle and expect both providers to be IDLE and the objective to be UNSET
  ASSERT_TRUE(objCtrl_->cycle());
  ASSERT_FALSE(objExeCtrlSvcP_->getCommandStatus());
  ASSERT_FALSE(objExeStateCtrlSvcP_->getCommandStatus());
  ASSERT_FALSE(objExeCtrlSvcP_->getActiveObjective());
}

TEST_F(ObjectiveControllerTest, ActiveConstraintFailed) {
  // This test will send an objective to the Objective Executor and walk it through an active constraint failure
  std::vector<std::shared_ptr<arlcore::umaa::conditional::ConditionalBase>> conditionals;
  UMAA::MM::Conditional::ConditionalType constraintConditional;
  constraintConditional.conditionalID(arlcore::UuidFactory::getInstance().generateGuid().getGuid());
  constraintConditional.name("Failed constraint conditional");
  std::shared_ptr<arlcore::umaa::conditional::ConditionalBase> conditionalPtr = std::make_shared<arlcore::test::MockConditional>(constraintConditional, false);
  conditionals.push_back(conditionalPtr);

  constraintsP_->update(conditionals);

  UMAA::MM::ActiveConstraintsControl::ActiveConstraintsCommandType activeConstraintsCommand;
  activeConstraintsCommand.constraintConditionalIDs().push_back(constraintConditional.conditionalID());
  activeConstraintsCommandReader_->send(activeConstraintsCommand);

  arlcore::umaa::ObjectiveType obj;
  obj.name() = "Generalized Objective 1";
  obj.objectiveID() = arlcore::UuidFactory::getInstance().generateGuid().getGuid();

  arlcore::umaa::ObjectiveExecutorCommandType objCmd;
  objCmd.objective() = obj;

  // Cycle dry-run - Providers should be IDLE and no active objective
  ASSERT_TRUE(objCtrl_->cycle());
  ASSERT_FALSE(objExeCtrlSvcP_->getCommandStatus());
  ASSERT_FALSE(objExeStateCtrlSvcP_->getCommandStatus());
  ASSERT_FALSE(objExeCtrlSvcP_->getActiveObjective());

  ASSERT_EQ(objCmdReader_->send(objCmd), SendStatus::SUCCESS);

  // Cycle and expect ObjectiveExecutorControlProvider to go to EXECUTING
  ASSERT_TRUE(objCtrl_->cycle());
  ASSERT_TRUE(objExeCtrlSvcP_->getCommandStatus());
  ASSERT_FALSE(objExeStateCtrlSvcP_->getCommandStatus());
  ASSERT_TRUE(objExeCtrlSvcP_->getActiveObjective());
  EXPECT_EQ(objExeCtrlSvcP_->getCommandStatus().value(), CommandStatusEnumType::EXECUTING);
  EXPECT_EQ(objExeCtrlSvcP_->getActiveObjective().value()->getName(), obj.name());
  EXPECT_EQ(objExeCtrlSvcP_->getActiveObjective().value()->getObjectiveState(),
    arlcore::umaa::ObjectiveExecutorStateEnumType::QUEUED);

  // Send an EXECUTE state command
  arlcore::umaa::ObjectiveExecutorStateCommandType objStateCmd;
  objStateCmd.objectiveID() = obj.objectiveID();
  objStateCmd.destination() = arlcore::umaa::IdentifierType(sourceId_.getGuid(), {});
  objStateCmd.objectiveState() = arlcore::umaa::ObjectiveExecutorControlEnumType::EXECUTE;
  objStateCmdReader_->send(objStateCmd);

  // Cycle and expect ObjectiveExecutorControlProvider to still be EXECUTING and objective to still be QUEUED.
  // The ObjectiveExecutorStateControlProvider will be ISSUED
  objStatSender_->clear();
  ASSERT_TRUE(objCtrl_->cycle());
  ASSERT_TRUE(objExeCtrlSvcP_->getCommandStatus());
  EXPECT_FALSE(objExeStateCtrlSvcP_->getCommandStatus());
  ASSERT_TRUE(objExeCtrlSvcP_->getActiveObjective());
  EXPECT_EQ(objExeCtrlSvcP_->getCommandStatus().value(), CommandStatusEnumType::EXECUTING);

  EXPECT_EQ(objExeCtrlSvcP_->getActiveObjective().value()->getObjectiveState(),
    arlcore::umaa::ObjectiveExecutorStateEnumType::FAILED);
  EXPECT_EQ(objExeCtrlSvcP_->getActiveObjective().value()->getObjectiveStateReason(),
    arlcore::umaa::ObjectiveExecutorStateReasonEnumType::CANNOT_PERFORM_UNDER_CONSTRAINTS);

  UMAA::MM::ObjectiveExecutorControl::ObjectiveExecutorStateCommandStatusType status;
  ASSERT_EQ(objStateStatSender_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::ISSUED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  ASSERT_EQ(objStateStatSender_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMMANDED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  ASSERT_EQ(objStateStatSender_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::EXECUTING);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  ASSERT_EQ(objStateStatSender_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMPLETED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  ASSERT_EQ(objStateStatSender_->read(&status), ReadStatus::DISPOSED);

  ASSERT_TRUE(objCtrl_->cycle());
  UMAA::MM::ObjectiveExecutorControl::ObjectiveExecutorCommandStatusType cmdStatus;
  ASSERT_EQ(objStatSender_->read(&cmdStatus), ReadStatus::SUCCESS);
  EXPECT_EQ(cmdStatus.commandStatus(), CommandStatusEnumType::FAILED);
  EXPECT_EQ(cmdStatus.commandStatusReason(), CommandStatusReasonEnumType::OBJECTIVE_FAILED);
  ASSERT_EQ(objStatSender_->read(&cmdStatus), ReadStatus::DISPOSED);

  // Cycle and expect both providers to be IDLE and the objective to be UNSET
  ASSERT_TRUE(objCtrl_->cycle());
  ASSERT_FALSE(objExeCtrlSvcP_->getCommandStatus());
  ASSERT_FALSE(objExeStateCtrlSvcP_->getCommandStatus());
  ASSERT_FALSE(objExeCtrlSvcP_->getActiveObjective());
}

TEST_F(ObjectiveControllerTest, CancelObjective) {
  arlcore::umaa::ObjectiveType obj;
  obj.name() = "Cancel Objective";
  obj.objectiveID() = arlcore::UuidFactory::getInstance().generateGuid().getGuid();

  arlcore::umaa::ObjectiveExecutorCommandType objCmd;
  objCmd.objective() = obj;

  // Cycle dry-run - Providers should be IDLE and no active objective
  ASSERT_TRUE(objCtrl_->cycle());
  ASSERT_FALSE(objExeCtrlSvcP_->getCommandStatus());
  ASSERT_FALSE(objExeStateCtrlSvcP_->getCommandStatus());
  ASSERT_FALSE(objExeCtrlSvcP_->getActiveObjective());

  ASSERT_EQ(objCmdReader_->send(objCmd), SendStatus::SUCCESS);

  // Cycle and expect ObjectiveExecutorControlProvider to go to EXECUTING
  ASSERT_TRUE(objCtrl_->cycle());
  ASSERT_TRUE(objExeCtrlSvcP_->getCommandStatus());
  ASSERT_FALSE(objExeStateCtrlSvcP_->getCommandStatus());
  ASSERT_TRUE(objExeCtrlSvcP_->getActiveObjective());
  EXPECT_EQ(objExeCtrlSvcP_->getCommandStatus().value(), CommandStatusEnumType::EXECUTING);
  EXPECT_EQ(objExeCtrlSvcP_->getActiveObjective().value()->getName(), obj.name());
  EXPECT_EQ(objExeCtrlSvcP_->getActiveObjective().value()->getObjectiveState(),
    arlcore::umaa::ObjectiveExecutorStateEnumType::QUEUED);

  // Dispose Objective command type to cancel
  ASSERT_EQ(objCmdReader_->dispose(objCmd), SendStatus::SUCCESS);

  // Cycle and expect ObjectiveExecutorControlProvider to go to straight to IDLE in one cycle
  // The objective state will be CANCELING and will stay there until the implementer moves it to CANCELED
  ASSERT_TRUE(objCtrl_->cycle());
  ASSERT_FALSE(objExeCtrlSvcP_->getCommandStatus());
  ASSERT_FALSE(objExeStateCtrlSvcP_->getCommandStatus());
  ASSERT_TRUE(objExeCtrlSvcP_->getActiveObjective());
  EXPECT_EQ(objExeCtrlSvcP_->getActiveObjective().value()->getObjectiveState(),
    arlcore::umaa::ObjectiveExecutorStateEnumType::CANCELING);

  ASSERT_TRUE(objExeCtrlSvcP_->getActiveObjective().value()->advanceObjectiveState());

  // One more cycle will move the the executor all the way back to IDLE and the active objective will have been reset
  // Since it was terminal
  ASSERT_TRUE(objCtrl_->cycle());
  ASSERT_FALSE(objExeCtrlSvcP_->getCommandStatus());
  ASSERT_FALSE(objExeStateCtrlSvcP_->getCommandStatus());
  ASSERT_FALSE(objExeCtrlSvcP_->getActiveObjective());
}

TEST_F(ObjectiveControllerTest, PauseAndResume) {
  arlcore::umaa::ObjectiveType obj;
  obj.name() = "Pause and Resume Ojective";
  obj.objectiveID() = arlcore::UuidFactory::getInstance().generateGuid().getGuid();

  arlcore::umaa::ObjectiveExecutorCommandType objCmd;
  objCmd.objective() = obj;

  // Cycle dry-run - Providers should be IDLE and no active objective
  ASSERT_TRUE(objCtrl_->cycle());
  ASSERT_FALSE(objExeCtrlSvcP_->getCommandStatus());
  ASSERT_FALSE(objExeStateCtrlSvcP_->getCommandStatus());
  ASSERT_FALSE(objExeCtrlSvcP_->getActiveObjective());

  ASSERT_EQ(objCmdReader_->send(objCmd), SendStatus::SUCCESS);

  // Cycle and expect ObjectiveExecutorControlProvider to go to EXECUTING
  ASSERT_TRUE(objCtrl_->cycle());
  ASSERT_TRUE(objExeCtrlSvcP_->getCommandStatus());
  ASSERT_FALSE(objExeStateCtrlSvcP_->getCommandStatus());
  ASSERT_TRUE(objExeCtrlSvcP_->getActiveObjective());
  EXPECT_EQ(objExeCtrlSvcP_->getCommandStatus().value(), CommandStatusEnumType::EXECUTING);
  EXPECT_EQ(objExeCtrlSvcP_->getActiveObjective().value()->getName(), obj.name());
  EXPECT_EQ(objExeCtrlSvcP_->getActiveObjective().value()->getObjectiveState(),
    arlcore::umaa::ObjectiveExecutorStateEnumType::QUEUED);

  // Set mock objective to never finish so we can test pausing and resuming
  if (auto MockObjective = std::dynamic_pointer_cast<arlcore::test::MockObjective>(
    objExeCtrlSvcP_->getActiveObjective().value())) {
      MockObjective->isObjectiveCompleteFlag = false;
  }

  // Send an EXECUTE state command
  arlcore::umaa::ObjectiveExecutorStateCommandType objStateCmd;
  objStateCmd.objectiveID() = obj.objectiveID();
  objStateCmd.destination() = arlcore::umaa::IdentifierType(sourceId_.getGuid(), {});
  objStateCmd.objectiveState() = arlcore::umaa::ObjectiveExecutorControlEnumType::EXECUTE;
  objStateCmdReader_->send(objStateCmd);

  // Cycle and expect ObjectiveExecutorControlProvider to still be EXECUTING and objective to still be QUEUED.
  // The ObjectiveExecutorStateControlProvider will be ISSUED
  ASSERT_TRUE(objCtrl_->cycle());
  ASSERT_TRUE(objExeCtrlSvcP_->getCommandStatus());
  ASSERT_FALSE(objExeStateCtrlSvcP_->getCommandStatus());
  ASSERT_TRUE(objExeCtrlSvcP_->getActiveObjective());
  EXPECT_EQ(objExeCtrlSvcP_->getCommandStatus().value(), CommandStatusEnumType::EXECUTING);

  EXPECT_EQ(objExeCtrlSvcP_->getActiveObjective().value()->getObjectiveState(),
    arlcore::umaa::ObjectiveExecutorStateEnumType::EXECUTING);

  UMAA::MM::ObjectiveExecutorControl::ObjectiveExecutorStateCommandStatusType status;
  ASSERT_EQ(objStateStatSender_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::ISSUED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  ASSERT_EQ(objStateStatSender_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMMANDED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  ASSERT_EQ(objStateStatSender_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::EXECUTING);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  ASSERT_EQ(objStateStatSender_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMPLETED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  ASSERT_EQ(objStateStatSender_->read(&status), ReadStatus::DISPOSED);

  // Cycle and expect ObjectiveExecutorControlProvider to be EXECUTING and objective to still be EXECUTING.
  // The ObjectiveExecutorStateControlProvider will be IDLE
  ASSERT_TRUE(objCtrl_->cycle());
  ASSERT_TRUE(objExeCtrlSvcP_->getCommandStatus());
  ASSERT_FALSE(objExeStateCtrlSvcP_->getCommandStatus());
  ASSERT_TRUE(objExeCtrlSvcP_->getActiveObjective());
  EXPECT_EQ(objExeCtrlSvcP_->getCommandStatus().value(), CommandStatusEnumType::EXECUTING);
  EXPECT_EQ(objExeCtrlSvcP_->getActiveObjective().value()->getObjectiveState(),
    arlcore::umaa::ObjectiveExecutorStateEnumType::EXECUTING);

  // Send a PAUSE state command
  objStateCmd.objectiveState() = arlcore::umaa::ObjectiveExecutorControlEnumType::PAUSE;
  objStateCmdReader_->send(objStateCmd);

  // Cycle and expect ObjectiveExecutorControlProvider to still be EXECUTING and objective to still be QUEUED.
  // The ObjectiveExecutorStateControlProvider will be ISSUED
  ASSERT_TRUE(objCtrl_->cycle());
  ASSERT_TRUE(objExeCtrlSvcP_->getCommandStatus());
  ASSERT_TRUE(objExeStateCtrlSvcP_->getCommandStatus());
  ASSERT_TRUE(objExeCtrlSvcP_->getActiveObjective());
  EXPECT_EQ(objExeCtrlSvcP_->getCommandStatus().value(), CommandStatusEnumType::EXECUTING);
  EXPECT_EQ(objExeStateCtrlSvcP_->getCommandStatus().value(), CommandStatusEnumType::EXECUTING);
  EXPECT_EQ(objExeCtrlSvcP_->getActiveObjective().value()->getObjectiveState(),
    arlcore::umaa::ObjectiveExecutorStateEnumType::PAUSING);
  objStateStatSender_->clear();

  // Move the objective to PAUSED
  ASSERT_TRUE(objExeCtrlSvcP_->getActiveObjective().value()->advanceObjectiveState());

  // Cycle and expect ObjectiveExecutorControlProvider to be EXECUTING and objective to be paused.
  // The ObjectiveExecutorStateControlProvider will be COMPLETED since the objective has reached the desired state
  ASSERT_TRUE(objCtrl_->cycle());
  ASSERT_TRUE(objExeCtrlSvcP_->getCommandStatus());
  ASSERT_FALSE(objExeStateCtrlSvcP_->getCommandStatus());
  ASSERT_TRUE(objExeCtrlSvcP_->getActiveObjective());
  EXPECT_EQ(objExeCtrlSvcP_->getCommandStatus().value(), CommandStatusEnumType::EXECUTING);
  EXPECT_EQ(objExeCtrlSvcP_->getActiveObjective().value()->getObjectiveState(),
    arlcore::umaa::ObjectiveExecutorStateEnumType::PAUSED);

  ASSERT_EQ(objStateStatSender_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMPLETED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  ASSERT_EQ(objStateStatSender_->read(&status), ReadStatus::DISPOSED);

  // Cycle and expect ObjectiveExecutorControlProvider to be EXECUTING and objective to still be PAUSED.
  // The ObjectiveExecutorStateControlProvider will be IDLE
  ASSERT_TRUE(objCtrl_->cycle());
  ASSERT_TRUE(objExeCtrlSvcP_->getCommandStatus());
  ASSERT_FALSE(objExeStateCtrlSvcP_->getCommandStatus());
  ASSERT_TRUE(objExeCtrlSvcP_->getActiveObjective());
  EXPECT_EQ(objExeCtrlSvcP_->getCommandStatus().value(), CommandStatusEnumType::EXECUTING);
  EXPECT_EQ(objExeCtrlSvcP_->getActiveObjective().value()->getObjectiveState(),
    arlcore::umaa::ObjectiveExecutorStateEnumType::PAUSED);

  // Send a RESUME state command
  objStateCmd.objectiveState() = arlcore::umaa::ObjectiveExecutorControlEnumType::RESUME;
  objStateCmdReader_->send(objStateCmd);

  // Cycle and expect ObjectiveExecutorControlProvider to be EXECUTING and objective to be PAUSED.
  // The ObjectiveExecutorStateControlProvider will be ISSUED
  ASSERT_TRUE(objCtrl_->cycle());
  ASSERT_TRUE(objExeCtrlSvcP_->getCommandStatus());
  ASSERT_TRUE(objExeStateCtrlSvcP_->getCommandStatus());
  ASSERT_TRUE(objExeCtrlSvcP_->getActiveObjective());
  EXPECT_EQ(objExeCtrlSvcP_->getCommandStatus().value(), CommandStatusEnumType::EXECUTING);
  EXPECT_EQ(objExeStateCtrlSvcP_->getCommandStatus().value(), CommandStatusEnumType::EXECUTING);
  EXPECT_EQ(objExeCtrlSvcP_->getActiveObjective().value()->getObjectiveState(),
    arlcore::umaa::ObjectiveExecutorStateEnumType::RESUMING);
  objStateStatSender_->clear();

  // Move the objective to EXECUTING and allow the objective to complete
  ASSERT_TRUE(objExeCtrlSvcP_->getActiveObjective().value()->advanceObjectiveState());

  // Cycle and expect ObjectiveExecutorControlProvider to be EXECUTING and objective to be EXECUTING.
  // The ObjectiveExecutorStateControlProvider will be COMPLETED
  ASSERT_TRUE(objCtrl_->cycle());
  ASSERT_TRUE(objExeCtrlSvcP_->getCommandStatus());
  ASSERT_FALSE(objExeStateCtrlSvcP_->getCommandStatus());
  ASSERT_TRUE(objExeCtrlSvcP_->getActiveObjective());
  EXPECT_EQ(objExeCtrlSvcP_->getCommandStatus().value(), CommandStatusEnumType::EXECUTING);
  EXPECT_EQ(objExeCtrlSvcP_->getActiveObjective().value()->getObjectiveState(),
    arlcore::umaa::ObjectiveExecutorStateEnumType::EXECUTING);

  ASSERT_EQ(objStateStatSender_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMPLETED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  ASSERT_EQ(objStateStatSender_->read(&status), ReadStatus::DISPOSED);

  // Cycle and expect ObjectiveExecutorControlProvider to be EXECUTING and objective to be RESUMING.
  // The ObjectiveExecutorStateControlProvider will be IDLE
  ASSERT_TRUE(objCtrl_->cycle());
  ASSERT_TRUE(objExeCtrlSvcP_->getCommandStatus());
  ASSERT_FALSE(objExeStateCtrlSvcP_->getCommandStatus());
  ASSERT_TRUE(objExeCtrlSvcP_->getActiveObjective());
  EXPECT_EQ(objExeCtrlSvcP_->getCommandStatus().value(), CommandStatusEnumType::EXECUTING);
  EXPECT_EQ(objExeCtrlSvcP_->getActiveObjective().value()->getObjectiveState(),
    arlcore::umaa::ObjectiveExecutorStateEnumType::EXECUTING);

  // Set mock objective to finish
  if (auto MockObjective = std::dynamic_pointer_cast<arlcore::test::MockObjective>(
    objExeCtrlSvcP_->getActiveObjective().value())) {
      MockObjective->isObjectiveCompleteFlag = true;
  }

  // Cycle and expect ObjectiveExecutorControlProvider to be EXECUTING and objective to be COMPLETED.
  // The ObjectiveExecutorStateControlProvider will be IDLE
  ASSERT_TRUE(objCtrl_->cycle());
  ASSERT_TRUE(objExeCtrlSvcP_->getCommandStatus());
  ASSERT_FALSE(objExeStateCtrlSvcP_->getCommandStatus());
  ASSERT_TRUE(objExeCtrlSvcP_->getActiveObjective());
  EXPECT_EQ(objExeCtrlSvcP_->getCommandStatus().value(), CommandStatusEnumType::EXECUTING);
  EXPECT_EQ(objExeCtrlSvcP_->getActiveObjective().value()->getObjectiveState(),
    arlcore::umaa::ObjectiveExecutorStateEnumType::COMPLETED);

  // Cycle and expect ObjectiveExecutorControlProvider to be IDLE and objective to be unset.
  // The ObjectiveExecutorStateControlProvider will be IDLE
  objStatSender_->clear();
  ASSERT_TRUE(objCtrl_->cycle());
  ASSERT_TRUE(objCtrl_->cycle());  // Takes an additional cycle to clear active objective
  ASSERT_FALSE(objExeCtrlSvcP_->getCommandStatus());
  ASSERT_FALSE(objExeStateCtrlSvcP_->getCommandStatus());
  ASSERT_FALSE(objExeCtrlSvcP_->getActiveObjective());

  UMAA::MM::ObjectiveExecutorControl::ObjectiveExecutorCommandStatusType cmdStatus;
  ASSERT_EQ(objStatSender_->read(&cmdStatus), ReadStatus::SUCCESS);
  EXPECT_EQ(cmdStatus.commandStatus(), CommandStatusEnumType::COMPLETED);
  EXPECT_EQ(cmdStatus.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  ASSERT_EQ(objStatSender_->read(&cmdStatus), ReadStatus::DISPOSED);
}

TEST_F(ObjectiveControllerTest, ModifyObjective) {
  arlcore::umaa::ObjectiveType obj;
  obj.name() = "Modify Objective";
  obj.objectiveID() = arlcore::UuidFactory::getInstance().generateGuid().getGuid();

  arlcore::umaa::ObjectiveExecutorCommandType objCmd;
  objCmd.objective() = obj;

  // Cycle dry-run - Providers should be IDLE and no active objective
  ASSERT_TRUE(objCtrl_->cycle());
  ASSERT_FALSE(objExeCtrlSvcP_->getCommandStatus());
  ASSERT_FALSE(objExeStateCtrlSvcP_->getCommandStatus());
  ASSERT_FALSE(objExeCtrlSvcP_->getActiveObjective());

  ASSERT_EQ(objCmdReader_->send(objCmd), SendStatus::SUCCESS);

  // Cycle and expect ObjectiveExecutorControlProvider to go to ISSUED
  ASSERT_TRUE(objCtrl_->cycle());
  ASSERT_TRUE(objExeCtrlSvcP_->getCommandStatus());
  ASSERT_FALSE(objExeStateCtrlSvcP_->getCommandStatus());
  ASSERT_TRUE(objExeCtrlSvcP_->getActiveObjective());
  EXPECT_EQ(objExeCtrlSvcP_->getCommandStatus().value(), CommandStatusEnumType::EXECUTING);
  EXPECT_EQ(objExeCtrlSvcP_->getActiveObjective().value()->getName(), obj.name());
  EXPECT_EQ(objExeCtrlSvcP_->getActiveObjective().value()->getObjectiveState(),
    arlcore::umaa::ObjectiveExecutorStateEnumType::QUEUED);

  // Set mock objective to never finish so we can test an update
  if (auto MockObjective = std::dynamic_pointer_cast<arlcore::test::MockObjective>(
    objExeCtrlSvcP_->getActiveObjective().value())) {
      MockObjective->isObjectiveCompleteFlag = false;
  }

  // Send an EXECUTE state command
  arlcore::umaa::ObjectiveExecutorStateCommandType objStateCmd;
  objStateCmd.objectiveID() = obj.objectiveID();
  objStateCmd.destination() = arlcore::umaa::IdentifierType(sourceId_.getGuid(), {});
  objStateCmd.objectiveState() = arlcore::umaa::ObjectiveExecutorControlEnumType::EXECUTE;
  objStateCmdReader_->send(objStateCmd);

  // Cycle and expect ObjectiveExecutorControlProvider to still be EXECUTING and objective to still be QUEUED.
  // The ObjectiveExecutorStateControlProvider will be ISSUED
  ASSERT_TRUE(objCtrl_->cycle());
  ASSERT_TRUE(objExeCtrlSvcP_->getCommandStatus());
  ASSERT_FALSE(objExeStateCtrlSvcP_->getCommandStatus());
  ASSERT_TRUE(objExeCtrlSvcP_->getActiveObjective());
  EXPECT_EQ(objExeCtrlSvcP_->getCommandStatus().value(), CommandStatusEnumType::EXECUTING);

  EXPECT_EQ(objExeCtrlSvcP_->getActiveObjective().value()->getObjectiveState(),
    arlcore::umaa::ObjectiveExecutorStateEnumType::EXECUTING);

  UMAA::MM::ObjectiveExecutorControl::ObjectiveExecutorStateCommandStatusType status;
  ASSERT_EQ(objStateStatSender_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::ISSUED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  ASSERT_EQ(objStateStatSender_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMMANDED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  ASSERT_EQ(objStateStatSender_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::EXECUTING);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  ASSERT_EQ(objStateStatSender_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMPLETED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  ASSERT_EQ(objStateStatSender_->read(&status), ReadStatus::DISPOSED);

  // Cycle and expect ObjectiveExecutorControlProvider to be EXECUTING and objective to still be EXECUTING.
  // The ObjectiveExecutorStateControlProvider will be IDLE
  ASSERT_TRUE(objCtrl_->cycle());
  ASSERT_TRUE(objExeCtrlSvcP_->getCommandStatus());
  ASSERT_FALSE(objExeStateCtrlSvcP_->getCommandStatus());
  ASSERT_TRUE(objExeCtrlSvcP_->getActiveObjective());
  EXPECT_EQ(objExeCtrlSvcP_->getCommandStatus().value(), CommandStatusEnumType::EXECUTING);
  EXPECT_EQ(objExeCtrlSvcP_->getActiveObjective().value()->getObjectiveState(),
    arlcore::umaa::ObjectiveExecutorStateEnumType::EXECUTING);

  obj.name() = "New Objective";
  objCmd.objective() = obj;
  ASSERT_EQ(objCmdReader_->send(objCmd), SendStatus::SUCCESS);

  // Cycle and expect ObjectiveExecutorControlProvider to go back to ISSUED and objective to still be MODIFYING.
  objStatSender_->clear();
  ASSERT_TRUE(objCtrl_->cycle());
  ASSERT_TRUE(objExeCtrlSvcP_->getCommandStatus());
  ASSERT_FALSE(objExeStateCtrlSvcP_->getCommandStatus());
  ASSERT_TRUE(objExeCtrlSvcP_->getActiveObjective());
  EXPECT_EQ(objExeCtrlSvcP_->getCommandStatus().value(), CommandStatusEnumType::EXECUTING);
  EXPECT_EQ(objExeCtrlSvcP_->getCommandStatusReason().value(), CommandStatusReasonEnumType::SUCCEEDED);
  EXPECT_EQ(objExeCtrlSvcP_->getActiveObjective().value()->getObjectiveState(),
    arlcore::umaa::ObjectiveExecutorStateEnumType::QUEUED);  // Modifying to queued in a single cycle

  UMAA::MM::ObjectiveExecutorControl::ObjectiveExecutorCommandStatusType cmdStatus;
  ASSERT_EQ(objStatSender_->read(&cmdStatus), ReadStatus::SUCCESS);
  EXPECT_EQ(cmdStatus.commandStatus(), CommandStatusEnumType::ISSUED);
  EXPECT_EQ(cmdStatus.commandStatusReason(), CommandStatusReasonEnumType::UPDATED);
  ASSERT_EQ(objStatSender_->read(&cmdStatus), ReadStatus::SUCCESS);
  EXPECT_EQ(cmdStatus.commandStatus(), CommandStatusEnumType::COMMANDED);
  EXPECT_EQ(cmdStatus.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  ASSERT_EQ(objStatSender_->read(&cmdStatus), ReadStatus::SUCCESS);
  EXPECT_EQ(cmdStatus.commandStatus(), CommandStatusEnumType::EXECUTING);
  EXPECT_EQ(cmdStatus.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);

  // Clean up
  ASSERT_EQ(objCmdReader_->dispose(objCmd), SendStatus::SUCCESS);

    // Cycle and expect ObjectiveExecutorControlProvider to IDLE and objective to be reset to QUEUED.
  ASSERT_TRUE(objCtrl_->cycle());
  ASSERT_FALSE(objExeCtrlSvcP_->getCommandStatus());
  ASSERT_FALSE(objExeStateCtrlSvcP_->getCommandStatus());
  ASSERT_TRUE(objExeCtrlSvcP_->getActiveObjective());
  EXPECT_EQ(objExeCtrlSvcP_->getActiveObjective().value()->getObjectiveState(),
    arlcore::umaa::ObjectiveExecutorStateEnumType::CANCELING);

  // Move the objective to CANCELED and allow the objective to complete
  ASSERT_TRUE(objExeCtrlSvcP_->getActiveObjective().value()->advanceObjectiveState());

    // Cycle and expect all to be IDLE
  ASSERT_TRUE(objCtrl_->cycle());
  ASSERT_FALSE(objExeCtrlSvcP_->getCommandStatus());
  ASSERT_FALSE(objExeStateCtrlSvcP_->getCommandStatus());
  ASSERT_FALSE(objExeCtrlSvcP_->getActiveObjective());
}

TEST_F(ObjectiveControllerTest, FailObjective) {
arlcore::umaa::ObjectiveType obj;
  obj.name() = "Fail Objective";
  obj.objectiveID() = arlcore::UuidFactory::getInstance().generateGuid().getGuid();

  arlcore::umaa::ObjectiveExecutorCommandType objCmd;
  objCmd.objective() = obj;

  // Cycle dry-run - Providers should be IDLE and no active objective
  ASSERT_TRUE(objCtrl_->cycle());
  ASSERT_FALSE(objExeCtrlSvcP_->getCommandStatus());
  ASSERT_FALSE(objExeStateCtrlSvcP_->getCommandStatus());
  ASSERT_FALSE(objExeCtrlSvcP_->getActiveObjective());

  ASSERT_EQ(objCmdReader_->send(objCmd), SendStatus::SUCCESS);

  // Cycle and expect ObjectiveExecutorControlProvider to go to ISSUED
  ASSERT_TRUE(objCtrl_->cycle());
  ASSERT_TRUE(objExeCtrlSvcP_->getCommandStatus());
  ASSERT_FALSE(objExeStateCtrlSvcP_->getCommandStatus());
  ASSERT_TRUE(objExeCtrlSvcP_->getActiveObjective());
  EXPECT_EQ(objExeCtrlSvcP_->getCommandStatus().value(), CommandStatusEnumType::EXECUTING);
  EXPECT_EQ(objExeCtrlSvcP_->getActiveObjective().value()->getName(), obj.name());
  EXPECT_EQ(objExeCtrlSvcP_->getActiveObjective().value()->getObjectiveState(),
    arlcore::umaa::ObjectiveExecutorStateEnumType::QUEUED);

  // Cycle and expect ObjectiveExecutorControlProvider to go to EXECUTING and objective to still be QUEUED.
  ASSERT_TRUE(objCtrl_->cycle());
  ASSERT_TRUE(objExeCtrlSvcP_->getCommandStatus());
  ASSERT_FALSE(objExeStateCtrlSvcP_->getCommandStatus());
  ASSERT_TRUE(objExeCtrlSvcP_->getActiveObjective());
  EXPECT_EQ(objExeCtrlSvcP_->getCommandStatus().value(), CommandStatusEnumType::EXECUTING);
  EXPECT_EQ(objExeCtrlSvcP_->getActiveObjective().value()->getObjectiveState(),
    arlcore::umaa::ObjectiveExecutorStateEnumType::QUEUED);

  // Set mock objective to never finish and set the fail flag to true
  if (auto MockObjective = std::dynamic_pointer_cast<arlcore::test::MockObjective>(
    objExeCtrlSvcP_->getActiveObjective().value())) {
      MockObjective->isObjectiveCompleteFlag = false;
      MockObjective->objectiveFailReason =
        arlcore::umaa::ObjectiveExecutorStateReasonEnumType::CANNOT_PERFORM_UNDER_CONSTRAINTS;
  }

  // Send an EXECUTE state command
  arlcore::umaa::ObjectiveExecutorStateCommandType objStateCmd;
  objStateCmd.objectiveID() = obj.objectiveID();
  objStateCmd.destination() = arlcore::umaa::IdentifierType(sourceId_.getGuid(), {});
  objStateCmd.objectiveState() = arlcore::umaa::ObjectiveExecutorControlEnumType::EXECUTE;
  objStateCmdReader_->send(objStateCmd);

  // Cycle and expect ObjectiveExecutorControlProvider to still be EXECUTING and objective to still be QUEUED.
  // The ObjectiveExecutorStateControlProvider will be ISSUED
  ASSERT_TRUE(objCtrl_->cycle());
  ASSERT_TRUE(objExeCtrlSvcP_->getCommandStatus());
  ASSERT_FALSE(objExeStateCtrlSvcP_->getCommandStatus());
  ASSERT_TRUE(objExeCtrlSvcP_->getActiveObjective());
  EXPECT_EQ(objExeCtrlSvcP_->getCommandStatus().value(), CommandStatusEnumType::EXECUTING);

  EXPECT_EQ(objExeCtrlSvcP_->getActiveObjective().value()->getObjectiveState(),
    arlcore::umaa::ObjectiveExecutorStateEnumType::FAILED);

  UMAA::MM::ObjectiveExecutorControl::ObjectiveExecutorStateCommandStatusType status;
  ASSERT_EQ(objStateStatSender_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::ISSUED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  ASSERT_EQ(objStateStatSender_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMMANDED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  ASSERT_EQ(objStateStatSender_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::EXECUTING);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  ASSERT_EQ(objStateStatSender_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMPLETED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  ASSERT_EQ(objStateStatSender_->read(&status), ReadStatus::DISPOSED);

  // Without an active objective the state command provider will also fail
  objStatSender_->clear();
  ASSERT_TRUE(objCtrl_->cycle());
  ASSERT_FALSE(objExeCtrlSvcP_->getCommandStatus());
  ASSERT_FALSE(objExeStateCtrlSvcP_->getCommandStatus());

  UMAA::MM::ObjectiveExecutorControl::ObjectiveExecutorCommandStatusType cmdStatus;
  ASSERT_EQ(objStatSender_->read(&cmdStatus), ReadStatus::SUCCESS);
  EXPECT_EQ(cmdStatus.commandStatus(), CommandStatusEnumType::FAILED);
  EXPECT_EQ(cmdStatus.commandStatusReason(), CommandStatusReasonEnumType::OBJECTIVE_FAILED);
  ASSERT_EQ(objStatSender_->read(&cmdStatus), ReadStatus::DISPOSED);

  // Finally all the providers will be back to IDLE
  ASSERT_TRUE(objCtrl_->cycle());
  ASSERT_FALSE(objExeCtrlSvcP_->getCommandStatus());
  ASSERT_FALSE(objExeStateCtrlSvcP_->getCommandStatus());
  ASSERT_FALSE(objExeCtrlSvcP_->getActiveObjective());
}

TEST_F(ObjectiveControllerTest, logReadStatus) {
  EXPECT_NO_THROW(objCtrl_->logReadStatus("", ReadStatus::DISPOSED));
  EXPECT_NO_THROW(objCtrl_->logReadStatus("", ReadStatus::ERROR));
  EXPECT_NO_THROW(objCtrl_->logReadStatus("", ReadStatus::INVALID_DATA));
  EXPECT_NO_THROW(objCtrl_->logReadStatus("", ReadStatus::NO_DATA));
  EXPECT_NO_THROW(objCtrl_->logReadStatus("", ReadStatus::NOT_IMPLEMENTED));
  EXPECT_NO_THROW(objCtrl_->logReadStatus("", ReadStatus::SUCCESS));
}
