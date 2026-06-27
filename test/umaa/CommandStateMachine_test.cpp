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

#include "CommandStateMachine.h"

using arlcore::umaa::CommandStateMachine;

class CommandStateMachineTest : public ::testing::Test {
 protected:
  void SetUp() override {
    state_ = std::make_shared<CommandStateMachine>();
  }

  void TearDown() override {
    // Resetting the shared pointer not the state machine so we can test CommandStateMachine's reset method
    state_.reset();
  }

  std::shared_ptr<CommandStateMachine> state_;
};

TEST_F(CommandStateMachineTest, isStateFinal) {
  EXPECT_FALSE(CommandStateMachine::isStateFinal(CommandStatusEnumType::ISSUED));
  EXPECT_FALSE(CommandStateMachine::isStateFinal(CommandStatusEnumType::COMMANDED));
  EXPECT_FALSE(CommandStateMachine::isStateFinal(CommandStatusEnumType::EXECUTING));
  EXPECT_TRUE(CommandStateMachine::isStateFinal(CommandStatusEnumType::COMPLETED));
  EXPECT_TRUE(CommandStateMachine::isStateFinal(CommandStatusEnumType::CANCELED));
  EXPECT_TRUE(CommandStateMachine::isStateFinal(CommandStatusEnumType::FAILED));
}

TEST_F(CommandStateMachineTest, allSuccess) {
  EXPECT_EQ(state_->getState(), CommandStatusEnumType::ISSUED);
  EXPECT_FALSE(state_->isFinal());
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), CommandStatusEnumType::COMMANDED);
  EXPECT_FALSE(state_->isFinal());
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), CommandStatusEnumType::EXECUTING);
  EXPECT_FALSE(state_->isFinal());
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), CommandStatusEnumType::COMPLETED);
  EXPECT_TRUE(state_->isFinal());

  // Make sure we can't go anywhere from a final state
  EXPECT_FALSE(state_->advanceState());
  EXPECT_FALSE(state_->update());
  EXPECT_FALSE(state_->fail(CommandStatusReasonEnumType::TIMEOUT));
  EXPECT_FALSE(state_->cancel());
}

TEST_F(CommandStateMachineTest, reset) {
  // Test from non-final states
  EXPECT_EQ(state_->getState(), CommandStatusEnumType::ISSUED);
  state_->reset();
  EXPECT_EQ(state_->getState(), CommandStatusEnumType::ISSUED);

  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), CommandStatusEnumType::COMMANDED);
  state_->reset();
  EXPECT_EQ(state_->getState(), CommandStatusEnumType::ISSUED);

  EXPECT_TRUE(state_->advanceState());
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), CommandStatusEnumType::EXECUTING);
  state_->reset();
  EXPECT_EQ(state_->getState(), CommandStatusEnumType::ISSUED);

  // Test from final states
  EXPECT_TRUE(state_->advanceState());
  EXPECT_TRUE(state_->advanceState());
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), CommandStatusEnumType::COMPLETED);
  EXPECT_TRUE(state_->isFinal());
  state_->reset();
  EXPECT_EQ(state_->getState(), CommandStatusEnumType::ISSUED);

  EXPECT_TRUE(state_->cancel());
  EXPECT_EQ(state_->getState(), CommandStatusEnumType::CANCELED);
  EXPECT_TRUE(state_->isFinal());
  state_->reset();
  EXPECT_EQ(state_->getState(), CommandStatusEnumType::ISSUED);

}

TEST_F(CommandStateMachineTest, update) {
  EXPECT_EQ(state_->getState(), CommandStatusEnumType::ISSUED);
  EXPECT_TRUE(state_->update());
  EXPECT_EQ(state_->getState(), CommandStatusEnumType::ISSUED);

  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), CommandStatusEnumType::COMMANDED);
  EXPECT_TRUE(state_->update());
  EXPECT_EQ(state_->getState(), CommandStatusEnumType::ISSUED);

  EXPECT_TRUE(state_->advanceState());
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), CommandStatusEnumType::EXECUTING);
  EXPECT_TRUE(state_->update());
  EXPECT_EQ(state_->getState(), CommandStatusEnumType::ISSUED);
}

TEST_F(CommandStateMachineTest, cancel) {
  EXPECT_EQ(state_->getState(), CommandStatusEnumType::ISSUED);
  EXPECT_TRUE(state_->cancel());
  EXPECT_EQ(state_->getState(), CommandStatusEnumType::CANCELED);
  state_->reset();

  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), CommandStatusEnumType::COMMANDED);
  EXPECT_TRUE(state_->cancel());
  EXPECT_EQ(state_->getState(), CommandStatusEnumType::CANCELED);
  state_->reset();

  EXPECT_TRUE(state_->advanceState());
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), CommandStatusEnumType::EXECUTING);
  EXPECT_TRUE(state_->cancel());
  EXPECT_EQ(state_->getState(), CommandStatusEnumType::CANCELED);
  state_->reset();

  EXPECT_TRUE(state_->advanceState());
  EXPECT_TRUE(state_->advanceState());
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), CommandStatusEnumType::COMPLETED);
  EXPECT_FALSE(state_->cancel());
  EXPECT_NE(state_->getState(), CommandStatusEnumType::CANCELED);
}

TEST_F(CommandStateMachineTest, fail) {
  EXPECT_EQ(state_->getState(), CommandStatusEnumType::ISSUED);

  // Not valid reason in any state
  EXPECT_FALSE(state_->fail(CommandStatusReasonEnumType::CANCELED));
  EXPECT_FALSE(state_->fail(CommandStatusReasonEnumType::UPDATED));
  EXPECT_FALSE(state_->fail(CommandStatusReasonEnumType::SUCCEEDED));

  // Not valid reason from this state
  EXPECT_FALSE(state_->fail(CommandStatusReasonEnumType::RESOURCE_REJECTED));
  EXPECT_FALSE(state_->fail(CommandStatusReasonEnumType::OBJECTIVE_FAILED));

  EXPECT_TRUE(state_->fail(CommandStatusReasonEnumType::VALIDATION_FAILED));
  EXPECT_EQ(state_->getState(), CommandStatusEnumType::FAILED);
  state_->reset();

  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), CommandStatusEnumType::COMMANDED);

  // Not valid reason from this state
  EXPECT_FALSE(state_->fail(CommandStatusReasonEnumType::VALIDATION_FAILED));
  EXPECT_FALSE(state_->fail(CommandStatusReasonEnumType::RESOURCE_FAILED));
  EXPECT_FALSE(state_->fail(CommandStatusReasonEnumType::OBJECTIVE_FAILED));

  EXPECT_TRUE(state_->fail(CommandStatusReasonEnumType::TIMEOUT));
  EXPECT_EQ(state_->getState(), CommandStatusEnumType::FAILED);
  state_->reset();

  EXPECT_TRUE(state_->advanceState());
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), CommandStatusEnumType::EXECUTING);

  // Not valid reason from this state
  EXPECT_FALSE(state_->fail(CommandStatusReasonEnumType::VALIDATION_FAILED));
  EXPECT_FALSE(state_->fail(CommandStatusReasonEnumType::RESOURCE_REJECTED));

  EXPECT_TRUE(state_->fail(CommandStatusReasonEnumType::INTERRUPTED));
  EXPECT_EQ(state_->getState(), CommandStatusEnumType::FAILED);
  state_->reset();

  EXPECT_TRUE(state_->advanceState());
  EXPECT_TRUE(state_->advanceState());
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), CommandStatusEnumType::COMPLETED);
  EXPECT_FALSE(state_->fail(CommandStatusReasonEnumType::SERVICE_FAILED));
  EXPECT_NE(state_->getState(), CommandStatusEnumType::CANCELED);
}