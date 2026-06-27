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

#include "AsyncCommandSession.h"

#include "UuidFactory.h"

using arlcore::umaa::domain::AsyncCommandSession;

class AsyncCommandSessionTest : public ::testing::Test {
 protected:
  static void SetUpTestSuite() {
    // Code to run at the start of the test suite
  }

  static void TearDownTestSuite() {
    // Code to run at the end of the test suite
  }

  void SetUp() override {
    // Code to run before running each TEST_F()
    p_ = std::promise<CommandStatusEnumType>();
    asyncCmdSess_ = std::make_unique<AsyncCommandSession>(p_.get_future());
  }

  void TearDown() override {
    // Code to run after running each TEST_F()
  }

  std::promise<CommandStatusEnumType> p_;
  std::unique_ptr<AsyncCommandSession> asyncCmdSess_;
};

void walkCommandStates(std::promise<CommandStatusEnumType>& p, AsyncCommandSession& s) {
    s.setSessionId(arlcore::UuidFactory::getInstance().generateGuid());
    s.setAsAcknowledged();
    s.setStatus(CommandStatusEnumType::ISSUED);
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    s.setStatus(CommandStatusEnumType::COMMANDED);
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    s.setStatus(CommandStatusEnumType::EXECUTING);
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    s.setStatus(CommandStatusEnumType::COMPLETED);
    p.set_value(CommandStatusEnumType::COMPLETED);
}

// Helper function to wait with a timeout
template <typename Callable, typename... Args>
bool callWithTimeout(Callable&& func, std::chrono::milliseconds timeout, Args&&... args) {
    std::packaged_task<void()> task(std::bind(std::forward<Callable>(func), std::forward<Args>(args)...));
    auto future = task.get_future();
    std::thread(std::move(task)).detach();
    return future.wait_for(timeout) == std::future_status::ready;
}

TEST_F(AsyncCommandSessionTest, Getters) {
  EXPECT_EQ(asyncCmdSess_->getSessionId(), arlcore::NIL_GUID);
  EXPECT_EQ(asyncCmdSess_->getStatus(), std::nullopt);
  EXPECT_FALSE(asyncCmdSess_->isCommandAcknowledged());
  EXPECT_FALSE(asyncCmdSess_->hasCancelBeenRequested());
}

TEST_F(AsyncCommandSessionTest, mutators) {
  asyncCmdSess_->setAsAcknowledged();
  EXPECT_TRUE(asyncCmdSess_->isCommandAcknowledged());

  auto testId = arlcore::UuidFactory::getInstance().generateGuid();
  asyncCmdSess_->setSessionId(testId);
  EXPECT_EQ(asyncCmdSess_->getSessionId(), testId);

  asyncCmdSess_->setStatus(CommandStatusEnumType::COMPLETED);
  EXPECT_EQ(asyncCmdSess_->getStatus(), CommandStatusEnumType::COMPLETED);

  asyncCmdSess_->requestCancel();
  EXPECT_TRUE(asyncCmdSess_->hasCancelBeenRequested());
}

TEST_F(AsyncCommandSessionTest, waitIndefFunctions) {
  std::thread t(walkCommandStates, std::ref(p_), std::ref(*asyncCmdSess_));

  // This test is exercising the functions that wait indefinitely.
  // Therefore, if this code changes, there is a chance the
  // unit tests will hang forever.

  EXPECT_NO_THROW(asyncCmdSess_->waitForAck());
  EXPECT_NO_THROW(asyncCmdSess_->waitForStatus(CommandStatusEnumType::COMMANDED));
  EXPECT_NO_THROW(asyncCmdSess_->wait());

  t.join();
}

TEST_F(AsyncCommandSessionTest, waitForFunctions) {
  EXPECT_FALSE(asyncCmdSess_->waitForAck_for(std::chrono::milliseconds(5)));
  EXPECT_FALSE(asyncCmdSess_->waitForStatus_for(CommandStatusEnumType::COMMANDED, std::chrono::milliseconds(5)));
  EXPECT_EQ(asyncCmdSess_->wait_for(std::chrono::milliseconds(5)), std::future_status::timeout);

  std::thread t(walkCommandStates, std::ref(p_), std::ref(*asyncCmdSess_));

  EXPECT_TRUE(asyncCmdSess_->waitForAck_for(std::chrono::milliseconds(100)));
  EXPECT_TRUE(asyncCmdSess_->waitForStatus_for(CommandStatusEnumType::COMMANDED, std::chrono::milliseconds(100)));
  EXPECT_EQ(asyncCmdSess_->wait_for(std::chrono::milliseconds(100)), std::future_status::ready);

  t.join();
}

TEST_F(AsyncCommandSessionTest, waitUntilFunctions) {
  std::chrono::steady_clock::time_point time = std::chrono::steady_clock::now() + std::chrono::milliseconds(5);
  EXPECT_FALSE(asyncCmdSess_->waitForAck_until(time));
  EXPECT_FALSE(asyncCmdSess_->waitForStatus_until(CommandStatusEnumType::COMMANDED, time));
  EXPECT_EQ(asyncCmdSess_->wait_until(time), std::future_status::timeout);

  std::thread t(walkCommandStates, std::ref(p_), std::ref(*asyncCmdSess_));
  time = std::chrono::steady_clock::now() + std::chrono::milliseconds(100);

  EXPECT_TRUE(asyncCmdSess_->waitForAck_until(time));
  EXPECT_TRUE(asyncCmdSess_->waitForStatus_until(CommandStatusEnumType::COMMANDED, time));
  EXPECT_EQ(asyncCmdSess_->wait_until(time), std::future_status::ready);

  t.join();
}
