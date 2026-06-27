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

#include "TaskStateMachine.h"

using arlcore::umaa::TaskStateMachine;

class TaskStateMachineTest : public ::testing::Test {
 protected:
  void SetUp() override {
    state_ = std::make_shared<TaskStateMachine>();
  }

  void TearDown() override {
    // resetting the shared pointer not the state machine so we can test TaskStateMachine's restart method
    state_.reset();
  }

  std::shared_ptr<TaskStateMachine> state_;
};

TEST_F(TaskStateMachineTest, isStateFinal) {
  EXPECT_FALSE(TaskStateMachine::isStateFinal(TaskStateEnumType::NOT_PLANNED));
  EXPECT_FALSE(TaskStateMachine::isStateFinal(TaskStateEnumType::PLANNING));
  EXPECT_FALSE(TaskStateMachine::isStateFinal(TaskStateEnumType::PLANNED));
  EXPECT_FALSE(TaskStateMachine::isStateFinal(TaskStateEnumType::AWAITING_EXECUTION_APPROVAL));
  EXPECT_FALSE(TaskStateMachine::isStateFinal(TaskStateEnumType::EXECUTION_APPROVED));
  EXPECT_FALSE(TaskStateMachine::isStateFinal(TaskStateEnumType::NOT_QUEUED));
  EXPECT_FALSE(TaskStateMachine::isStateFinal(TaskStateEnumType::QUEUING));
  EXPECT_FALSE(TaskStateMachine::isStateFinal(TaskStateEnumType::QUEUED));
  EXPECT_FALSE(TaskStateMachine::isStateFinal(TaskStateEnumType::EXECUTING));
  EXPECT_FALSE(TaskStateMachine::isStateFinal(TaskStateEnumType::CANCELING));
  EXPECT_FALSE(TaskStateMachine::isStateFinal(TaskStateEnumType::PAUSING));
  EXPECT_FALSE(TaskStateMachine::isStateFinal(TaskStateEnumType::PAUSED));
  EXPECT_FALSE(TaskStateMachine::isStateFinal(TaskStateEnumType::RESUMING));
  EXPECT_FALSE(TaskStateMachine::isStateFinal(TaskStateEnumType::RESTARTING));
  EXPECT_TRUE(TaskStateMachine::isStateFinal(TaskStateEnumType::COMPLETED));
  EXPECT_TRUE(TaskStateMachine::isStateFinal(TaskStateEnumType::CANCELED));
  EXPECT_TRUE(TaskStateMachine::isStateFinal(TaskStateEnumType::FAILED));
}

TEST_F(TaskStateMachineTest, allSuccess) {
  EXPECT_EQ(state_->getState(), TaskStateEnumType::NOT_PLANNED);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::PLANNING);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::PLANNED);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::AWAITING_EXECUTION_APPROVAL);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::EXECUTION_APPROVED);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::NOT_QUEUED);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::QUEUING);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::QUEUED);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::EXECUTING);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::COMPLETED);

  // Make sure we can't go anywhere from a final state
  EXPECT_FALSE(state_->advanceState());
  EXPECT_FALSE(state_->update());
  EXPECT_FALSE(state_->fail());
  EXPECT_FALSE(state_->cancel());
}

TEST_F(TaskStateMachineTest, restart) {
  // Test from non-final states
  EXPECT_EQ(state_->getState(), TaskStateEnumType::NOT_PLANNED);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::PLANNING);
  EXPECT_TRUE(state_->restart());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::RESTARTING);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::NOT_PLANNED);

  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::PLANNING);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::PLANNED);
  EXPECT_TRUE(state_->restart());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::RESTARTING);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::NOT_PLANNED);

  EXPECT_TRUE(state_->advanceState()); // PLANNING
  EXPECT_TRUE(state_->advanceState()); // PLANNED
  EXPECT_TRUE(state_->advanceState()); // AWAITING_EXECUTION_APPROVAL
  EXPECT_TRUE(state_->advanceState()); // EXECUTION_APPROVED
  EXPECT_TRUE(state_->advanceState()); // NOT_QUEUED
  EXPECT_TRUE(state_->advanceState()); // QUEUING
  EXPECT_TRUE(state_->advanceState()); // QUEUED
  EXPECT_TRUE(state_->advanceState()); // EXECUTING
  EXPECT_EQ(state_->getState(), TaskStateEnumType::EXECUTING);
  EXPECT_TRUE(state_->restart());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::RESTARTING);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::NOT_PLANNED);

  // Test from final states - Just Completed for now
  EXPECT_TRUE(state_->advanceState()); // PLANNING
  EXPECT_TRUE(state_->advanceState()); // PLANNED
  EXPECT_TRUE(state_->advanceState()); // AWAITING_EXECUTION_APPROVAL
  EXPECT_TRUE(state_->advanceState()); // EXECUTION_APPROVED
  EXPECT_TRUE(state_->advanceState()); // NOT_QUEUED
  EXPECT_TRUE(state_->advanceState()); // QUEUING
  EXPECT_TRUE(state_->advanceState()); // QUEUED
  EXPECT_TRUE(state_->advanceState()); // EXECUTING
  EXPECT_TRUE(state_->advanceState()); // COMPLETED
  EXPECT_EQ(state_->getState(), TaskStateEnumType::COMPLETED);
  EXPECT_TRUE(state_->restart());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::RESTARTING);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::NOT_PLANNED);

  // Test from not-planned state
  EXPECT_FALSE(state_->restart());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::NOT_PLANNED);
}

TEST_F(TaskStateMachineTest, update) {
  EXPECT_EQ(state_->getState(), TaskStateEnumType::NOT_PLANNED);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::PLANNING);
  EXPECT_TRUE(state_->update());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::NOT_PLANNED);

  // Go to executing
  EXPECT_TRUE(state_->advanceState()); // PLANNING
  EXPECT_TRUE(state_->advanceState()); // PLANNED
  EXPECT_TRUE(state_->advanceState()); // AWAITING_EXECUTION_APPROVAL
  EXPECT_TRUE(state_->advanceState()); // EXECUTION_APPROVED
  EXPECT_TRUE(state_->advanceState()); // NOT_QUEUED
  EXPECT_TRUE(state_->advanceState()); // QUEUING
  EXPECT_TRUE(state_->advanceState()); // QUEUED
  EXPECT_TRUE(state_->advanceState()); // EXECUTING
  EXPECT_TRUE(state_->update());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::NOT_PLANNED);

  // Can't update if in final state
  EXPECT_TRUE(state_->advanceState()); // PLANNING
  EXPECT_TRUE(state_->advanceState()); // PLANNED
  EXPECT_TRUE(state_->advanceState()); // AWAITING_EXECUTION_APPROVAL
  EXPECT_TRUE(state_->advanceState()); // EXECUTION_APPROVED
  EXPECT_TRUE(state_->advanceState()); // NOT_QUEUED
  EXPECT_TRUE(state_->advanceState()); // QUEUING
  EXPECT_TRUE(state_->advanceState()); // QUEUED
  EXPECT_TRUE(state_->advanceState()); // EXECUTING
  EXPECT_TRUE(state_->advanceState()); // COMPLETED
  EXPECT_FALSE(state_->update());
  EXPECT_TRUE(state_->restart());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::RESTARTING);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::NOT_PLANNED);

  // Can't update if canceling
  EXPECT_TRUE(state_->advanceState()); // PLANNING
  EXPECT_TRUE(state_->cancel());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::CANCELING);
  EXPECT_FALSE(state_->update());
  EXPECT_TRUE(state_->restart());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::RESTARTING);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::NOT_PLANNED);

  // Can't update if paused
  EXPECT_TRUE(state_->advanceState()); // PLANNING
  EXPECT_TRUE(state_->pause());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::PAUSING);
  EXPECT_FALSE(state_->update());
  EXPECT_TRUE(state_->restart());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::RESTARTING);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::NOT_PLANNED);

  // Can't update if restarting
  EXPECT_TRUE(state_->advanceState()); // PLANNING
  EXPECT_TRUE(state_->restart());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::RESTARTING);
  EXPECT_FALSE(state_->update());
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::NOT_PLANNED);
}

TEST_F(TaskStateMachineTest, pauseAndResume) {
  EXPECT_EQ(state_->getState(), TaskStateEnumType::NOT_PLANNED);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::PLANNING);
  EXPECT_TRUE(state_->pause());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::PAUSING);

  // Can't pause, resume, or update while PAUSING
  EXPECT_FALSE(state_->pause());
  EXPECT_FALSE(state_->resume());
  EXPECT_FALSE(state_->update());

  EXPECT_TRUE(state_->advanceState());

  EXPECT_EQ(state_->getState(), TaskStateEnumType::PAUSED);

  // Can't pause, advance, or update while PAUSED
  EXPECT_FALSE(state_->pause());
  EXPECT_FALSE(state_->advanceState());
  EXPECT_FALSE(state_->update());

  EXPECT_TRUE(state_->resume());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::RESUMING);

  // Can't pause, resume, or update while RESUMING
  EXPECT_FALSE(state_->pause());
  EXPECT_FALSE(state_->resume());
  EXPECT_FALSE(state_->update());

  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::NOT_PLANNED);
}

TEST_F(TaskStateMachineTest, cancel) {
  // Cancel from NOT_PLANNED
  EXPECT_EQ(state_->getState(), TaskStateEnumType::NOT_PLANNED);
  EXPECT_TRUE(state_->cancel());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::CANCELING);

  // Can only advance to CANCELED from CANCELING
  EXPECT_FALSE(state_->cancel());
  EXPECT_FALSE(state_->pause());
  EXPECT_FALSE(state_->resume());
  EXPECT_FALSE(state_->update());

  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::CANCELED);
  EXPECT_TRUE(state_->restart());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::RESTARTING);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::NOT_PLANNED);

  // Cancel from EXECUTING
  EXPECT_TRUE(state_->advanceState()); // PLANNING
  EXPECT_TRUE(state_->advanceState()); // PLANNED
  EXPECT_TRUE(state_->advanceState()); // AWAITING_EXECUTION_APPROVAL
  EXPECT_TRUE(state_->advanceState()); // EXECUTION_APPROVED
  EXPECT_TRUE(state_->advanceState()); // NOT_QUEUED
  EXPECT_TRUE(state_->advanceState()); // QUEUING
  EXPECT_TRUE(state_->advanceState()); // QUEUED
  EXPECT_TRUE(state_->advanceState()); // EXECUTING
  EXPECT_EQ(state_->getState(), TaskStateEnumType::EXECUTING);
  EXPECT_TRUE(state_->cancel());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::CANCELING);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::CANCELED);
  EXPECT_TRUE(state_->restart());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::RESTARTING);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::NOT_PLANNED);

  // Cancel from PAUSING
  EXPECT_TRUE(state_->advanceState()); // PLANNING
  EXPECT_TRUE(state_->pause());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::PAUSING);
  EXPECT_TRUE(state_->cancel());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::CANCELING);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::CANCELED);
  EXPECT_TRUE(state_->restart());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::RESTARTING);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::NOT_PLANNED);

  // Cancel from PAUSED
  EXPECT_TRUE(state_->advanceState()); // PLANNING
  EXPECT_TRUE(state_->pause());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::PAUSING);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::PAUSED);
  EXPECT_TRUE(state_->cancel());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::CANCELING);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::CANCELED);
  EXPECT_TRUE(state_->restart());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::RESTARTING);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::NOT_PLANNED);

  // Cancel from RESUMING
  EXPECT_TRUE(state_->advanceState()); // PLANNING
  EXPECT_TRUE(state_->pause());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::PAUSING);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::PAUSED);
  EXPECT_TRUE(state_->resume());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::RESUMING);
  EXPECT_TRUE(state_->cancel());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::CANCELING);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::CANCELED);
  EXPECT_TRUE(state_->restart());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::RESTARTING);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::NOT_PLANNED);

  // Can't cancel from final state
  EXPECT_TRUE(state_->advanceState()); // PLANNING
  EXPECT_TRUE(state_->advanceState()); // PLANNED
  EXPECT_TRUE(state_->advanceState()); // AWAITING_EXECUTION_APPROVAL
  EXPECT_TRUE(state_->advanceState()); // EXECUTION_APPROVED
  EXPECT_TRUE(state_->advanceState()); // NOT_QUEUED
  EXPECT_TRUE(state_->advanceState()); // QUEUING
  EXPECT_TRUE(state_->advanceState()); // QUEUED
  EXPECT_TRUE(state_->advanceState()); // EXECUTING
  EXPECT_TRUE(state_->advanceState()); // COMPLETED
  EXPECT_EQ(state_->getState(), TaskStateEnumType::COMPLETED);
  EXPECT_FALSE(state_->cancel());
  EXPECT_NE(state_->getState(), TaskStateEnumType::CANCELING);
}

TEST_F(TaskStateMachineTest, fail) {
  EXPECT_EQ(state_->getState(), TaskStateEnumType::NOT_PLANNED);

  // Fail from NOT_PLANNED
  EXPECT_TRUE(state_->fail());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::FAILED);
  EXPECT_TRUE(state_->restart());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::RESTARTING);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::NOT_PLANNED);

  // Fail from EXECUTING
  EXPECT_TRUE(state_->advanceState()); // PLANNING
  EXPECT_TRUE(state_->advanceState()); // PLANNED
  EXPECT_TRUE(state_->advanceState()); // AWAITING_EXECUTION_APPROVAL
  EXPECT_TRUE(state_->advanceState()); // EXECUTION_APPROVED
  EXPECT_TRUE(state_->advanceState()); // NOT_QUEUED
  EXPECT_TRUE(state_->advanceState()); // QUEUING
  EXPECT_TRUE(state_->advanceState()); // QUEUED
  EXPECT_TRUE(state_->advanceState()); // EXECUTING
  EXPECT_EQ(state_->getState(), TaskStateEnumType::EXECUTING);
  EXPECT_TRUE(state_->fail());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::FAILED);
  EXPECT_TRUE(state_->restart());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::RESTARTING);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::NOT_PLANNED);

  // Fail from PAUSING
  EXPECT_TRUE(state_->advanceState()); // PLANNING
  EXPECT_TRUE(state_->pause()); // PAUSING
  EXPECT_EQ(state_->getState(), TaskStateEnumType::PAUSING);
  EXPECT_TRUE(state_->fail());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::FAILED);
  EXPECT_TRUE(state_->restart());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::RESTARTING);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::NOT_PLANNED);

  // Fail from PAUSED
  EXPECT_TRUE(state_->advanceState()); // PLANNING
  EXPECT_TRUE(state_->pause()); // PAUSING
  EXPECT_TRUE(state_->advanceState()); // PAUSED
  EXPECT_EQ(state_->getState(), TaskStateEnumType::PAUSED);
  EXPECT_TRUE(state_->fail());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::FAILED);
  EXPECT_TRUE(state_->restart());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::RESTARTING);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::NOT_PLANNED);

  // Fail from RESUMING
  EXPECT_TRUE(state_->advanceState()); // PLANNING
  EXPECT_TRUE(state_->pause()); // PAUSING
  EXPECT_TRUE(state_->advanceState()); // PAUSED
  EXPECT_TRUE(state_->resume());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::RESUMING);
  EXPECT_TRUE(state_->fail());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::FAILED);
  EXPECT_TRUE(state_->restart());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::RESTARTING);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::NOT_PLANNED);

  // Can't fail from CANCELING
  EXPECT_TRUE(state_->advanceState()); // PLANNING
  EXPECT_TRUE(state_->cancel());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::CANCELING);
  EXPECT_FALSE(state_->fail());
  EXPECT_TRUE(state_->restart());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::RESTARTING);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::NOT_PLANNED);

  // Can't fail from FAILED
  EXPECT_TRUE(state_->advanceState()); // PLANNING
  EXPECT_TRUE(state_->fail());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::FAILED);
  EXPECT_FALSE(state_->fail());
  EXPECT_TRUE(state_->restart());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::RESTARTING);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::NOT_PLANNED);

  // Can't fail from final state
  EXPECT_TRUE(state_->advanceState()); // PLANNING
  EXPECT_TRUE(state_->advanceState()); // PLANNED
  EXPECT_TRUE(state_->advanceState()); // AWAITING_EXECUTION_APPROVAL
  EXPECT_TRUE(state_->advanceState()); // EXECUTION_APPROVED
  EXPECT_TRUE(state_->advanceState()); // NOT_QUEUED
  EXPECT_TRUE(state_->advanceState()); // QUEUING
  EXPECT_TRUE(state_->advanceState()); // QUEUED
  EXPECT_TRUE(state_->advanceState()); // EXECUTING
  EXPECT_TRUE(state_->advanceState()); // COMPLETED
  EXPECT_EQ(state_->getState(), TaskStateEnumType::COMPLETED);
  EXPECT_FALSE(state_->fail());
  EXPECT_TRUE(state_->restart());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::RESTARTING);
  EXPECT_TRUE(state_->advanceState());
  EXPECT_EQ(state_->getState(), TaskStateEnumType::NOT_PLANNED);
}