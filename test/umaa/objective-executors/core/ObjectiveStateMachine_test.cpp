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

#include "ObjectiveStateMachine.h"

using arlcore::umaa::ObjectiveStateMachine;

class ObjectiveStateMachineTest : public ::testing::Test {
 protected:
  void SetUp() override {
    state_ = std::make_shared<ObjectiveStateMachine>();
  }

  void TearDown() override {
    // Resetting the shared pointer not the state machine so we can test ObjectiveStateMachine's reset method
    state_.reset();
  }

  std::shared_ptr<ObjectiveStateMachine> state_;
};

TEST_F(ObjectiveStateMachineTest, isStateFinal) {
  EXPECT_FALSE(ObjectiveStateMachine::isStateFinal(arlcore::umaa::ObjectiveExecutorStateEnumType::QUEUED));
  EXPECT_FALSE(ObjectiveStateMachine::isStateFinal(arlcore::umaa::ObjectiveExecutorStateEnumType::EXECUTING));
  EXPECT_FALSE(ObjectiveStateMachine::isStateFinal(arlcore::umaa::ObjectiveExecutorStateEnumType::MODIFYING));
  EXPECT_FALSE(ObjectiveStateMachine::isStateFinal(arlcore::umaa::ObjectiveExecutorStateEnumType::PAUSING));
  EXPECT_FALSE(ObjectiveStateMachine::isStateFinal(arlcore::umaa::ObjectiveExecutorStateEnumType::PAUSED));
  EXPECT_FALSE(ObjectiveStateMachine::isStateFinal(arlcore::umaa::ObjectiveExecutorStateEnumType::RESUMING));
  EXPECT_FALSE(ObjectiveStateMachine::isStateFinal(arlcore::umaa::ObjectiveExecutorStateEnumType::CANCELING));
  EXPECT_TRUE(ObjectiveStateMachine::isStateFinal(arlcore::umaa::ObjectiveExecutorStateEnumType::COMPLETED));
  EXPECT_TRUE(ObjectiveStateMachine::isStateFinal(arlcore::umaa::ObjectiveExecutorStateEnumType::CANCELED));
  EXPECT_TRUE(ObjectiveStateMachine::isStateFinal(arlcore::umaa::ObjectiveExecutorStateEnumType::FAILED));
}

TEST_F(ObjectiveStateMachineTest, allSuccess) {
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::QUEUED);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::EXECUTING);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::COMPLETED);

  // Make sure we can't go anywhere from a final state
  EXPECT_FALSE(state_->advanceState());
  EXPECT_FALSE(state_->update());
  EXPECT_FALSE(state_->fail(arlcore::umaa::ObjectiveExecutorStateReasonEnumType::INTERNAL_FAILURE));
  EXPECT_FALSE(state_->cancel());
}

TEST_F(ObjectiveStateMachineTest, reset) {
  // Test from non-final states
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::QUEUED);
  state_->reset();
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::QUEUED);

  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::EXECUTING);
  state_->reset();
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::QUEUED);

  EXPECT_TRUE(state_->advanceState());
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::COMPLETED);
  state_->reset();
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::QUEUED);

  // Test from final states
  state_->reset();
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::QUEUED);
  EXPECT_TRUE(state_->cancel());
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::CANCELING);
  state_->reset();
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::QUEUED);

}

TEST_F(ObjectiveStateMachineTest, update) {
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::QUEUED);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::EXECUTING);
  EXPECT_TRUE(state_->update());
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::MODIFYING);

  // Can't pause, resume, or update while updating
  EXPECT_FALSE(state_->pause());
  EXPECT_FALSE(state_->resume());
  EXPECT_FALSE(state_->update());

  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::EXECUTING);
}

TEST_F(ObjectiveStateMachineTest, pause) {
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::QUEUED);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::EXECUTING);
  EXPECT_TRUE(state_->pause());
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::PAUSING);

  // Can't pause, resume, or update while PAUSING
  EXPECT_FALSE(state_->pause());
  EXPECT_FALSE(state_->resume());
  EXPECT_FALSE(state_->update());

  EXPECT_TRUE(state_->advanceState());

  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::PAUSED);

  // Can't pause, advance, or update while PAUSED
  EXPECT_FALSE(state_->pause());
  EXPECT_FALSE(state_->advanceState());
  EXPECT_FALSE(state_->update());

  EXPECT_TRUE(state_->resume());
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::RESUMING);

  // Can't pause, resume, or update while RESUMING
  EXPECT_FALSE(state_->pause());
  EXPECT_FALSE(state_->resume());
  EXPECT_FALSE(state_->update());

  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::EXECUTING);
}

TEST_F(ObjectiveStateMachineTest, cancel) {
  // Cancel from QUEUED
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::QUEUED);
  EXPECT_TRUE(state_->cancel());
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::CANCELING);

  // Can only advance to CANCELED from CANCELING
  EXPECT_FALSE(state_->cancel());
  EXPECT_FALSE(state_->pause());
  EXPECT_FALSE(state_->resume());
  EXPECT_FALSE(state_->update());

  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::CANCELED);
  state_->reset();

  // Cancel from EXECUTING
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::EXECUTING);
  EXPECT_TRUE(state_->cancel());
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::CANCELING);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::CANCELED);
  state_->reset();

  // Cancel from MODIFYING
  EXPECT_TRUE(state_->advanceState());
  EXPECT_TRUE(state_->update());
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::MODIFYING);
  EXPECT_TRUE(state_->cancel());
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::CANCELING);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::CANCELED);
  state_->reset();

  // Cancel from PAUSING
  EXPECT_TRUE(state_->advanceState());
  EXPECT_TRUE(state_->pause());
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::PAUSING);
  EXPECT_TRUE(state_->cancel());
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::CANCELING);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::CANCELED);
  state_->reset();

  // Cancel from PAUSED
  EXPECT_TRUE(state_->advanceState());
  EXPECT_TRUE(state_->pause());
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::PAUSED);
  EXPECT_TRUE(state_->cancel());
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::CANCELING);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::CANCELED);
  state_->reset();

  // Cancel from RESUMING
  EXPECT_TRUE(state_->advanceState());
  EXPECT_TRUE(state_->pause());
  EXPECT_TRUE(state_->advanceState());
  EXPECT_TRUE(state_->resume());
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::RESUMING);
  EXPECT_TRUE(state_->cancel());
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::CANCELING);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::CANCELED);
  state_->reset();

  // Can't cancel from final state
  EXPECT_TRUE(state_->advanceState());
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::COMPLETED);
  EXPECT_FALSE(state_->cancel());
  EXPECT_NE(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::CANCELING);
}

TEST_F(ObjectiveStateMachineTest, fail) {
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::QUEUED);

  // Not valid reason in any state
  EXPECT_FALSE(state_->fail(arlcore::umaa::ObjectiveExecutorStateReasonEnumType::BUS_MSG_DISPOSE));
  EXPECT_FALSE(state_->fail(arlcore::umaa::ObjectiveExecutorStateReasonEnumType::BUS_MSG_UPDATE));
  EXPECT_FALSE(state_->fail(arlcore::umaa::ObjectiveExecutorStateReasonEnumType::SUCCEEDED));
  EXPECT_FALSE(state_->fail(arlcore::umaa::ObjectiveExecutorStateReasonEnumType::COMMANDED));

  // Fail from QUEUED
  EXPECT_TRUE(state_->fail(arlcore::umaa::ObjectiveExecutorStateReasonEnumType::CANNOT_PERFORM_UNDER_CONSTRAINTS));
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::FAILED);
  state_->reset();

  // Fail from EXECUTING
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::EXECUTING);
  EXPECT_TRUE(state_->fail(arlcore::umaa::ObjectiveExecutorStateReasonEnumType::INTERNAL_FAILURE));
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::FAILED);
  state_->reset();

  // Fail from MODIFYING
  EXPECT_TRUE(state_->advanceState());
  EXPECT_TRUE(state_->update());
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::MODIFYING);
  EXPECT_TRUE(state_->fail(arlcore::umaa::ObjectiveExecutorStateReasonEnumType::LOWER_SERVICE_FAILED));
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::FAILED);
  state_->reset();

  // Fail from PAUSING
  EXPECT_TRUE(state_->advanceState());
  EXPECT_TRUE(state_->pause());
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::PAUSING);
  EXPECT_TRUE(state_->fail(arlcore::umaa::ObjectiveExecutorStateReasonEnumType::LOWER_SERVICE_TIMEOUT));
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::FAILED);
  state_->reset();

  // Fail from PAUSED
  EXPECT_TRUE(state_->advanceState());
  EXPECT_TRUE(state_->pause());
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::PAUSED);
  EXPECT_TRUE(state_->fail(arlcore::umaa::ObjectiveExecutorStateReasonEnumType::LOWER_SERVICE_REJECTED));
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::FAILED);
  state_->reset();

  // Fail from RESUMING
  EXPECT_TRUE(state_->advanceState());
  EXPECT_TRUE(state_->pause());
  EXPECT_TRUE(state_->advanceState());
  EXPECT_TRUE(state_->resume());
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::RESUMING);
  EXPECT_TRUE(state_->fail(arlcore::umaa::ObjectiveExecutorStateReasonEnumType::LOWER_SERVICE_INTERRUPTED));
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::FAILED);
  state_->reset();

  // Can't fail from FAILED
  EXPECT_TRUE(state_->fail(arlcore::umaa::ObjectiveExecutorStateReasonEnumType::COMMAND_VALIDATION_FAILED));
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::FAILED);
  EXPECT_FALSE(state_->fail(arlcore::umaa::ObjectiveExecutorStateReasonEnumType::COMMAND_VALIDATION_FAILED));
  state_->reset();

  // Can't fail from final state
  EXPECT_TRUE(state_->advanceState());
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), arlcore::umaa::ObjectiveExecutorStateEnumType::COMPLETED);
  EXPECT_FALSE(state_->fail(arlcore::umaa::ObjectiveExecutorStateReasonEnumType::OBJECTIVE_REPLACED));
  state_->reset();
}