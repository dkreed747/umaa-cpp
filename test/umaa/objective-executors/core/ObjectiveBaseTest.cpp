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

#include "ObjectiveBase.h"
#include "MockConditional.h"
#include "UuidFactory.h"

class testObjective : public arlcore::umaa::ObjectiveBase {
 public:
  explicit testObjective(
    const arlcore::umaa::ObjectiveType& obj,
    std::shared_ptr<arlcore::io::SenderBase<arlcore::umaa::ObjectiveExecutorExecutionStatusReportType>> exeStatSender) :
    ObjectiveBase(obj, exeStatSender) {}

  bool onCycle() override { return true;}
  bool onQueued() override { return true; }
  bool onCanceled() override { return true; }
  bool onCanceling() override { return true; }
  bool onCompleted() override { return true; }
  bool onExecuting() override { return true; }
  bool onFailed() override { return true; }
  bool onModifying() override { return true; }
  bool onPaused() override { return true; }
  bool onPausing() override { return true; }
  bool onResuming() override { return true; }

  bool isObjectiveValid() override { return true; }
  bool isObjectiveComplete() override { return true; }
  arlcore::umaa::ObjectiveExecutorStateReasonEnumType isObjectiveFailed() override {
    return arlcore::umaa::ObjectiveExecutorStateReasonEnumType::SUCCEEDED;
  }

  std::optional<std::vector<std::shared_ptr<arlcore::umaa::conditional::ConditionalBase>>> getActiveConstraints() {
    return this->activeConstraints_;
  }

  arlcore::io::SendStatus sendExecutionStatus() { return arlcore::io::SendStatus::NOT_IMPLEMENTED; }
};

class ObjectiveBaseTest : public ::testing::Test {
 protected:
  static void SetUpTestSuite() {
    obj_.approvalRequired() = true;
    obj_.duringConditionID() = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    obj_.name() = "TestName";
    obj_.objectiveDescription() = "Test Objective";
    obj_.objectiveID() = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    obj_.objectivePriority() = 255;
    obj_.preconditionID() = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    std::vector<arlcore::umaa::IdentifierType> resourceIds = {
      arlcore::umaa::IdentifierType({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}, {}),
      arlcore::umaa::IdentifierType({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}, {}),
      arlcore::umaa::IdentifierType({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}, {})};
    obj_.preferredResourceID() = resourceIds;
    obj_.specializationID() = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    obj_.specializationTimestamp() = UMAA::Common::Measurement::DateTime(5, 5);
    obj_.specializationTopic() = "UMAA::Types::SpecializationTypesTopic";
    std::vector<arlcore::umaa::StateTriggerType> stateTriggers = {
      arlcore::umaa::StateTriggerType(
        {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16},
        2,
        UMAA::Common::MaritimeEnumeration::TriggerStateEnumModule::TriggerStateEnumType::PLAN
      ),
      arlcore::umaa::StateTriggerType(
        {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16},
        5,
        UMAA::Common::MaritimeEnumeration::TriggerStateEnumModule::TriggerStateEnumType::QUEUE
      )
    };
    obj_.stateTrigger() = stateTriggers;
  }

  static UMAA::MM::BaseType::ObjectiveType obj_;
};

UMAA::MM::BaseType::ObjectiveType ObjectiveBaseTest::obj_;

TEST_F(ObjectiveBaseTest, dataAccessors) {
  testObjective tObj(obj_, nullptr);

  EXPECT_EQ(tObj.isApprovalRequired(), obj_.approvalRequired());
  EXPECT_EQ(tObj.getDuringConditionID().value(), obj_.duringConditionID().value());
  EXPECT_EQ(tObj.getName(), obj_.name());
  EXPECT_EQ(tObj.getDescription(), obj_.objectiveDescription());
  EXPECT_EQ(tObj.getId(), obj_.objectiveID());
  EXPECT_EQ(tObj.getPriority(), obj_.objectivePriority());
  EXPECT_EQ(tObj.getPreconditionID().value(), obj_.preconditionID().value());
  ASSERT_EQ(tObj.getPreferredResources().size(), obj_.preferredResourceID().size());
  EXPECT_EQ(tObj.getPreferredResources().front(), obj_.preferredResourceID().front());
  EXPECT_EQ(tObj.getSpecializationID(), obj_.specializationID());
  EXPECT_EQ(tObj.getSpecializationTimestamp(), obj_.specializationTimestamp());
  EXPECT_EQ(tObj.getSpecializationTopic(), obj_.specializationTopic());
  ASSERT_EQ(tObj.getStateTriggers().size(), obj_.stateTrigger().size());
  EXPECT_EQ(tObj.getStateTriggers().front(), obj_.stateTrigger().front());

  EXPECT_EQ(tObj.getObjectiveState(), arlcore::umaa::ObjectiveExecutorStateEnumType::QUEUED);
  EXPECT_EQ(tObj.getObjectiveStateReason(), arlcore::umaa::ObjectiveExecutorStateReasonEnumType::SUCCEEDED);
  EXPECT_FALSE(tObj.isObjectiveInTerminalState());

  ASSERT_TRUE(tObj.advanceObjectiveState());
  EXPECT_EQ(tObj.getObjectiveState(), arlcore::umaa::ObjectiveExecutorStateEnumType::EXECUTING);
  EXPECT_EQ(tObj.getObjectiveStateReason(), arlcore::umaa::ObjectiveExecutorStateReasonEnumType::SUCCEEDED);
}

TEST_F(ObjectiveBaseTest, updateConstraints) {
  std::vector<std::shared_ptr<arlcore::umaa::conditional::ConditionalBase>> constraints;
  UMAA::MM::Conditional::ConditionalType conditional;
  conditional.conditionalID(arlcore::UuidFactory::getInstance().generateGuid().getGuid());
  conditional.name("Test Conditional");
  std::shared_ptr<arlcore::umaa::conditional::ConditionalBase> constraint = std::make_shared<arlcore::test::MockConditional>(conditional, true);
  constraints.push_back(constraint);

  testObjective tObj(obj_, nullptr);
  tObj.update(constraints);
  auto activeConstraints = tObj.getActiveConstraints();
  EXPECT_EQ(constraints, activeConstraints);
}

TEST_F(ObjectiveBaseTest, commandObjectiveState) {
  testObjective tObj(obj_, nullptr);

  // This test walks through the state tree by testing the commandStateObjectiveFunction

  // We cannot pause or resume from the QUEUED state --> should return false
  ASSERT_FALSE(tObj.commandObjectiveState(
    arlcore::umaa::ObjectiveExecutorControlEnumType::PAUSE));
  ASSERT_FALSE(tObj.commandObjectiveState(
    arlcore::umaa::ObjectiveExecutorControlEnumType::RESUME));
  EXPECT_EQ(tObj.getObjectiveState(), arlcore::umaa::ObjectiveExecutorStateEnumType::QUEUED);
  EXPECT_EQ(tObj.getObjectiveStateReason(), arlcore::umaa::ObjectiveExecutorStateReasonEnumType::SUCCEEDED);

  // Move state to executing
  ASSERT_TRUE(tObj.commandObjectiveState(
    arlcore::umaa::ObjectiveExecutorControlEnumType::EXECUTE));
  EXPECT_EQ(tObj.getObjectiveState(), arlcore::umaa::ObjectiveExecutorStateEnumType::EXECUTING);
  EXPECT_EQ(tObj.getObjectiveStateReason(), arlcore::umaa::ObjectiveExecutorStateReasonEnumType::SUCCEEDED);

  // Pause the Objective
  ASSERT_TRUE(tObj.commandObjectiveState(
    arlcore::umaa::ObjectiveExecutorControlEnumType::PAUSE));
  EXPECT_EQ(tObj.getObjectiveState(), arlcore::umaa::ObjectiveExecutorStateEnumType::PAUSING);
  EXPECT_EQ(tObj.getObjectiveStateReason(), arlcore::umaa::ObjectiveExecutorStateReasonEnumType::COMMANDED);

  // Need to advance state once to get out of the in progress PAUSING state
  ASSERT_TRUE(tObj.advanceObjectiveState());
  EXPECT_EQ(tObj.getObjectiveState(), arlcore::umaa::ObjectiveExecutorStateEnumType::PAUSED);
  EXPECT_EQ(tObj.getObjectiveStateReason(), arlcore::umaa::ObjectiveExecutorStateReasonEnumType::SUCCEEDED);

  // Resume the Objective
  ASSERT_TRUE(tObj.commandObjectiveState(
    arlcore::umaa::ObjectiveExecutorControlEnumType::RESUME));
  EXPECT_EQ(tObj.getObjectiveState(), arlcore::umaa::ObjectiveExecutorStateEnumType::RESUMING);
  EXPECT_EQ(tObj.getObjectiveStateReason(), arlcore::umaa::ObjectiveExecutorStateReasonEnumType::COMMANDED);

  // Need to advance state once to get out of the in progress RESUMING state
  ASSERT_TRUE(tObj.advanceObjectiveState());
  EXPECT_EQ(tObj.getObjectiveState(), arlcore::umaa::ObjectiveExecutorStateEnumType::EXECUTING);
  EXPECT_EQ(tObj.getObjectiveStateReason(), arlcore::umaa::ObjectiveExecutorStateReasonEnumType::SUCCEEDED);
}
