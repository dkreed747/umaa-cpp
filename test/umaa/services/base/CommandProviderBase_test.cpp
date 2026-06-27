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
#include <UMAA/MO/GlobalVectorControl/GlobalVectorCommandType.hpp>
#include <UMAA/MO/GlobalVectorControl/GlobalVectorCommandAckReportType.hpp>
#include <UMAA/MO/GlobalVectorControl/GlobalVectorCommandStatusType.hpp>
#include <UMAA/MO/GlobalVectorControl/GlobalVectorExecutionStatusReportType.hpp>

#include "CommandProviderBase.h"
#include "LocalReaderSender.h"
#include "UuidFactory.h"

using UMAA::MO::GlobalVectorControl::GlobalVectorCommandAckReportType;
using UMAA::MO::GlobalVectorControl::GlobalVectorCommandStatusType;
using UMAA::MO::GlobalVectorControl::GlobalVectorCommandType;
using UMAA::MO::GlobalVectorControl::GlobalVectorExecutionStatusReportType;
using arlcore::io::LocalReaderSender;
using arlcore::io::ReadStatus;
using arlcore::io::SendStatus;

using GlobalVectorCommandProviderBase =
    arlcore::umaa::services::CommandProviderBase<GlobalVectorCommandType, GlobalVectorCommandAckReportType,
                                                 GlobalVectorCommandStatusType, GlobalVectorExecutionStatusReportType>;
using GlobalVectorCommandProviderIo =
    arlcore::umaa::domain::UmaaCommandProviderIo<GlobalVectorCommandType, GlobalVectorCommandAckReportType,
                                             GlobalVectorCommandStatusType, GlobalVectorExecutionStatusReportType>;

using arlcore::umaa::services::CommandStateResult;

class TestCommandProvider : public GlobalVectorCommandProviderBase {
 public:
  TestCommandProvider(const NumericGuid& source, std::shared_ptr<GlobalVectorCommandProviderIo> io)
      : GlobalVectorCommandProviderBase(source, io) {}

  bool isCommandValid(const GlobalVectorCommandType& cmd) override {
    NumericGuid id(cmd.sessionID());
    return cmdValid.count(id) == 0 ? true : cmdValid.at(id);
  }

  bool isCommandCompleted(const std::weak_ptr<CmdSession> session) override {
    if (auto cmdSession = session.lock(); cmdComplete.count(cmdSession->getSessionId()) > 0) {
      return cmdComplete.at(cmdSession->getSessionId());
    }
    return false;
  }

  CommandStatusReasonEnumType isCommandFailed(const std::weak_ptr<CmdSession> session) override {
    if (auto cmdSession = session.lock(); cmdFailed.count(cmdSession->getSessionId()) > 0) {
      return cmdFailed.at(cmdSession->getSessionId());
    }
    return CommandStatusReasonEnumType::SUCCEEDED;
  }

  CommandStateResult onCommanded(const std::weak_ptr<CmdSession> session) override {
    if (auto cmdSession = session.lock(); onCommandedReturn.count(cmdSession->getSessionId()) > 0) {
      return onCommandedReturn.at(cmdSession->getSessionId());
    }
    return CommandStateResult::ADVANCE;
  }

  SendStatus sendExecutionStatus(const GlobalVectorCommandType& cmd) override {
    UMAA_LOG_TRACE(util::SYSTEM_LOGGER, "Sending Exe Status")
    NumericGuid id(cmd.sessionID());
    GlobalVectorExecutionStatusReportType r;
    r.source().id(this->source_.getGuid());
    r.sessionID(cmd.sessionID());
    bool complete = cmdComplete.count(id) ? cmdComplete.at(id) : false;
    r.elevationAchieved(complete);
    r.speedAchieved(complete);
    r.timeStamp(arlcore::umaa::getTimestamp());
    return this->io_->cmdExeStatusSender.value()->send(r);
  }

  SendStatus disposeExecutionStatus(const GlobalVectorCommandType& cmd) override {
    UMAA_LOG_TRACE(util::SYSTEM_LOGGER, "Disposing Exe Status")
    GlobalVectorExecutionStatusReportType r;
    r.source().id(this->source_.getGuid());
    r.sessionID(cmd.sessionID());
    r.timeStamp(arlcore::umaa::getTimestamp());
    return this->io_->cmdExeStatusSender.value()->dispose(r);
  }

  std::map<NumericGuid, bool> cmdComplete;
  std::map<NumericGuid, bool> cmdValid;
  std::map<NumericGuid, CommandStateResult> onCommandedReturn;
  std::map<NumericGuid, CommandStatusReasonEnumType> cmdFailed;
};

class CommandProviderBaseTest : public ::testing::Test {
 protected:
  static void SetUpTestSuite() {
    // Code to run at the start of the test suite
    // Set values of static variables
    commandIo_ = std::make_shared<LocalReaderSender<GlobalVectorCommandType>>();
    commandAckIo_ = std::make_shared<LocalReaderSender<GlobalVectorCommandAckReportType>>();
    cmdStatusIo_ = std::make_shared<LocalReaderSender<GlobalVectorCommandStatusType>>();
    cmdExeStatusIo_ = std::make_shared<LocalReaderSender<GlobalVectorExecutionStatusReportType>>();
    io_ = std::make_shared<GlobalVectorCommandProviderIo>(commandIo_, commandAckIo_, cmdStatusIo_, cmdExeStatusIo_);
  }

  static void TearDownTestSuite() {
    // Code to run at the end of the test suite
  }

  void SetUp() override {
    cmdProv = std::make_shared<TestCommandProvider>(provider_id_, io_);
  }

  void TearDown() override {
    cmdProv.reset();
    commandIo_->clear();
    commandAckIo_->clear();
    cmdStatusIo_->clear();
    cmdExeStatusIo_->clear();
  }

  // Static Declarations
  static arlcore::NumericGuid provider_id_;
  static arlcore::NumericGuid consumer_id_;
  static std::shared_ptr<GlobalVectorCommandProviderIo> io_;
  static std::shared_ptr<LocalReaderSender<GlobalVectorCommandType>> commandIo_;
  static std::shared_ptr<LocalReaderSender<GlobalVectorCommandAckReportType>> commandAckIo_;
  static std::shared_ptr<LocalReaderSender<GlobalVectorCommandStatusType>> cmdStatusIo_;
  static std::shared_ptr<LocalReaderSender<GlobalVectorExecutionStatusReportType>> cmdExeStatusIo_;

  std::shared_ptr<TestCommandProvider> cmdProv = nullptr;
};

// Static Definitions
arlcore::NumericGuid CommandProviderBaseTest::provider_id_ = arlcore::UuidFactory::getInstance().generateGuid();
arlcore::NumericGuid CommandProviderBaseTest::consumer_id_ = arlcore::UuidFactory::getInstance().generateGuid();
std::shared_ptr<GlobalVectorCommandProviderIo> CommandProviderBaseTest::io_ = nullptr;
std::shared_ptr<LocalReaderSender<GlobalVectorCommandType>> CommandProviderBaseTest::commandIo_ = nullptr;
std::shared_ptr<LocalReaderSender<GlobalVectorCommandAckReportType>> CommandProviderBaseTest::commandAckIo_ = nullptr;
std::shared_ptr<LocalReaderSender<GlobalVectorCommandStatusType>> CommandProviderBaseTest::cmdStatusIo_ = nullptr;
std::shared_ptr<LocalReaderSender<GlobalVectorExecutionStatusReportType>> CommandProviderBaseTest::cmdExeStatusIo_ = nullptr;

TEST_F(CommandProviderBaseTest, noOverrides) {
  GlobalVectorCommandProviderBase provider(provider_id_, io_);

  NumericGuid sessionId = arlcore::UuidFactory::getInstance().generateGuid();
  GlobalVectorCommandType cmd;
  GlobalVectorCommandStatusType status;
  GlobalVectorCommandAckReportType ack;
  GlobalVectorExecutionStatusReportType report;
  cmd.source().id(consumer_id_.getGuid());
  cmd.destination().id(provider_id_.getGuid());
  cmd.sessionID(sessionId.getGuid());

  commandIo_->send(cmd);
  EXPECT_TRUE(cmdProv->cycle());

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::ISSUED);

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMMANDED);

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::EXECUTING);

  commandIo_->send(cmd);  // Update
  EXPECT_TRUE(cmdProv->cycle());

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::ISSUED);

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMMANDED);

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::EXECUTING);

  EXPECT_TRUE(cmdProv->cycle());
  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::NO_DATA);

  commandIo_->dispose(cmd);  // Cancel
  EXPECT_TRUE(cmdProv->cycle());
  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::CANCELED);

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::DISPOSED);
}

TEST_F(CommandProviderBaseTest, commandComplete) {
  NumericGuid sessionId = arlcore::UuidFactory::getInstance().generateGuid();
  UMAA::Common::Measurement::DateTime nullTimestamp(0, 0);
  GlobalVectorCommandType cmd;
  GlobalVectorCommandStatusType status;
  GlobalVectorCommandAckReportType ack;
  GlobalVectorExecutionStatusReportType report;
  cmd.source().id(consumer_id_.getGuid());
  cmd.destination().id(provider_id_.getGuid());
  cmd.sessionID(sessionId.getGuid());

  commandIo_->send(cmd);
  EXPECT_TRUE(cmdProv->cycle());

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::ISSUED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  EXPECT_NE(status.timeStamp(), nullTimestamp);

  EXPECT_EQ(commandAckIo_->read(&ack), ReadStatus::SUCCESS);
  EXPECT_EQ(ack.sessionID(), sessionId.getGuid());
  EXPECT_EQ(ack.command(), cmd);
  EXPECT_EQ(ack.source().id(), provider_id_.getGuid());
  EXPECT_NE(ack.timeStamp(), nullTimestamp);

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMMANDED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::EXECUTING);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);

  EXPECT_EQ(cmdExeStatusIo_->read(&report), ReadStatus::SUCCESS);
  EXPECT_EQ(report.sessionID(), sessionId.getGuid());
  EXPECT_EQ(report.source().id(), provider_id_.getGuid());
  EXPECT_FALSE(report.elevationAchieved());
  EXPECT_FALSE(report.speedAchieved());
  EXPECT_NE(report.timeStamp(), nullTimestamp);

  EXPECT_TRUE(cmdProv->cycle());
  EXPECT_EQ(cmdStatusIo_->count(), 0);
  EXPECT_EQ(cmdExeStatusIo_->read(&report), ReadStatus::SUCCESS);
  EXPECT_EQ(report.sessionID(), sessionId.getGuid());
  EXPECT_EQ(report.source().id(), provider_id_.getGuid());
  EXPECT_FALSE(report.elevationAchieved());
  EXPECT_FALSE(report.speedAchieved());

  cmdProv->cmdComplete[sessionId] = true;
  EXPECT_TRUE(cmdProv->cycle());
  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMPLETED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  EXPECT_EQ(cmdExeStatusIo_->read(&report), ReadStatus::SUCCESS);
  EXPECT_EQ(report.sessionID(), sessionId.getGuid());
  EXPECT_EQ(report.source().id(), provider_id_.getGuid());
  EXPECT_TRUE(report.elevationAchieved());
  EXPECT_TRUE(report.speedAchieved());

  // Check that all status and reports were disposed
  EXPECT_EQ(commandAckIo_->read(&ack), ReadStatus::DISPOSED);
  EXPECT_EQ(ack.sessionID(), sessionId.getGuid());
  EXPECT_EQ(ack.source().id(), provider_id_.getGuid());
  EXPECT_NE(ack.timeStamp(), nullTimestamp);

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::DISPOSED);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.source().id(), provider_id_.getGuid());
  EXPECT_NE(status.timeStamp(), nullTimestamp);

  EXPECT_EQ(cmdExeStatusIo_->readLatest(&report), ReadStatus::DISPOSED);
  EXPECT_EQ(report.sessionID(), sessionId.getGuid());
  EXPECT_EQ(report.source().id(), provider_id_.getGuid());
  EXPECT_NE(report.timeStamp(), nullTimestamp);

  EXPECT_TRUE(cmdProv->cycle());
  EXPECT_EQ(cmdStatusIo_->count(), 0);
}

TEST_F(CommandProviderBaseTest, commandUpdate) {
  NumericGuid sessionId = arlcore::UuidFactory::getInstance().generateGuid();
  GlobalVectorCommandType cmd;
  GlobalVectorCommandStatusType status;
  GlobalVectorCommandAckReportType ack;
  GlobalVectorExecutionStatusReportType report;
  cmd.source().id(consumer_id_.getGuid());
  cmd.sessionID(sessionId.getGuid());
  commandIo_->send(cmd);
  EXPECT_TRUE(cmdProv->cycle());

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::ISSUED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  EXPECT_EQ(commandAckIo_->read(&ack), ReadStatus::SUCCESS);
  EXPECT_EQ(ack.sessionID(), sessionId.getGuid());
  EXPECT_EQ(ack.command(), cmd);
  EXPECT_EQ(ack.source().id(), provider_id_.getGuid());

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMMANDED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::EXECUTING);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);

  EXPECT_EQ(cmdStatusIo_->count(), 0);
  commandIo_->send(cmd);
  EXPECT_TRUE(cmdProv->cycle());
  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::ISSUED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::UPDATED);

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMMANDED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::EXECUTING);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  EXPECT_EQ(cmdExeStatusIo_->read(&report), ReadStatus::SUCCESS);
  EXPECT_EQ(report.sessionID(), sessionId.getGuid());
  EXPECT_EQ(report.source().id(), provider_id_.getGuid());
  EXPECT_FALSE(report.elevationAchieved());
  EXPECT_FALSE(report.speedAchieved());

  EXPECT_EQ(cmdStatusIo_->count(), 0);
  EXPECT_EQ(cmdExeStatusIo_->read(&report), ReadStatus::SUCCESS);
  EXPECT_EQ(report.sessionID(), sessionId.getGuid());
  EXPECT_EQ(report.source().id(), provider_id_.getGuid());
  EXPECT_FALSE(report.elevationAchieved());
  EXPECT_FALSE(report.speedAchieved());

  cmdProv->cmdComplete[sessionId] = true;
  EXPECT_TRUE(cmdProv->cycle());
  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMPLETED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  EXPECT_EQ(cmdExeStatusIo_->read(&report), ReadStatus::SUCCESS);
  EXPECT_EQ(report.sessionID(), sessionId.getGuid());
  EXPECT_EQ(report.source().id(), provider_id_.getGuid());
  EXPECT_TRUE(report.elevationAchieved());
  EXPECT_TRUE(report.speedAchieved());

  // Check that all status and reports were disposed
  EXPECT_EQ(commandAckIo_->read(&ack), ReadStatus::DISPOSED);
  EXPECT_EQ(ack.sessionID(), sessionId.getGuid());
  EXPECT_EQ(ack.source().id(), provider_id_.getGuid());
  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::DISPOSED);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.source().id(), provider_id_.getGuid());
  EXPECT_EQ(cmdExeStatusIo_->readLatest(&report), ReadStatus::DISPOSED);
  EXPECT_EQ(report.sessionID(), sessionId.getGuid());
  EXPECT_EQ(report.source().id(), provider_id_.getGuid());

  EXPECT_TRUE(cmdProv->cycle());
  EXPECT_EQ(cmdStatusIo_->count(), 0);
}

TEST_F(CommandProviderBaseTest, commandCancel) {
  NumericGuid sessionId = arlcore::UuidFactory::getInstance().generateGuid();
  GlobalVectorCommandType cmd;
  GlobalVectorCommandStatusType status;
  GlobalVectorCommandAckReportType ack;
  GlobalVectorExecutionStatusReportType report;
  cmd.source().id(consumer_id_.getGuid());
  cmd.sessionID(sessionId.getGuid());
  commandIo_->send(cmd);
  cmdProv->onCommandedReturn[sessionId] = CommandStateResult::OK;
  EXPECT_TRUE(cmdProv->cycle());

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::ISSUED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  EXPECT_EQ(commandAckIo_->read(&ack), ReadStatus::SUCCESS);
  EXPECT_EQ(ack.sessionID(), sessionId.getGuid());
  EXPECT_EQ(ack.command(), cmd);
  EXPECT_EQ(ack.source().id(), provider_id_.getGuid());

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMMANDED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);

  EXPECT_EQ(cmdStatusIo_->count(), 0);
  commandIo_->dispose(cmd);
  EXPECT_TRUE(cmdProv->cycle());
  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::CANCELED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::CANCELED);

  // Check that all status and reports were disposed
  EXPECT_EQ(commandAckIo_->read(&ack), ReadStatus::DISPOSED);
  EXPECT_EQ(ack.sessionID(), sessionId.getGuid());
  EXPECT_EQ(ack.source().id(), provider_id_.getGuid());
  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::DISPOSED);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.source().id(), provider_id_.getGuid());

  EXPECT_TRUE(cmdProv->cycle());
  EXPECT_EQ(cmdStatusIo_->count(), 0);
}

TEST_F(CommandProviderBaseTest, onCommandFailed) {
  NumericGuid sessionId = arlcore::UuidFactory::getInstance().generateGuid();
  GlobalVectorCommandType cmd;
  GlobalVectorCommandStatusType status;
  GlobalVectorCommandAckReportType ack;
  GlobalVectorExecutionStatusReportType report;
  cmd.source().id(consumer_id_.getGuid());
  cmd.sessionID(sessionId.getGuid());
  commandIo_->send(cmd);
  cmdProv->onCommandedReturn[sessionId] = CommandStateResult::ERROR;
  EXPECT_TRUE(cmdProv->cycle());

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::ISSUED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  EXPECT_EQ(commandAckIo_->read(&ack), ReadStatus::SUCCESS);
  EXPECT_EQ(ack.sessionID(), sessionId.getGuid());
  EXPECT_EQ(ack.command(), cmd);
  EXPECT_EQ(ack.source().id(), provider_id_.getGuid());

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMMANDED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::FAILED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SERVICE_FAILED);

  // Check that all status and reports were disposed
  EXPECT_EQ(commandAckIo_->read(&ack), ReadStatus::DISPOSED);
  EXPECT_EQ(ack.sessionID(), sessionId.getGuid());
  EXPECT_EQ(ack.source().id(), provider_id_.getGuid());
  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::DISPOSED);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.source().id(), provider_id_.getGuid());

  EXPECT_TRUE(cmdProv->cycle());
  EXPECT_EQ(cmdStatusIo_->count(), 0);
}

TEST_F(CommandProviderBaseTest, commandInvalid) {
  NumericGuid sessionId = arlcore::UuidFactory::getInstance().generateGuid();
  GlobalVectorCommandType cmd;
  GlobalVectorCommandStatusType status;
  GlobalVectorCommandAckReportType ack;
  GlobalVectorExecutionStatusReportType report;
  cmd.source().id(consumer_id_.getGuid());
  cmd.sessionID(sessionId.getGuid());
  commandIo_->send(cmd);
  cmdProv->cmdValid[sessionId] = false;
  EXPECT_TRUE(cmdProv->cycle());

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::ISSUED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  EXPECT_EQ(commandAckIo_->read(&ack), ReadStatus::SUCCESS);
  EXPECT_EQ(ack.sessionID(), sessionId.getGuid());
  EXPECT_EQ(ack.command(), cmd);
  EXPECT_EQ(ack.source().id(), provider_id_.getGuid());

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::FAILED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::VALIDATION_FAILED);

  // Check that all status and reports were disposed
  EXPECT_EQ(commandAckIo_->read(&ack), ReadStatus::DISPOSED);
  EXPECT_EQ(ack.sessionID(), sessionId.getGuid());
  EXPECT_EQ(ack.source().id(), provider_id_.getGuid());
  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::DISPOSED);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.source().id(), provider_id_.getGuid());

  EXPECT_TRUE(cmdProv->cycle());
  EXPECT_EQ(cmdStatusIo_->count(), 0);
}

TEST_F(CommandProviderBaseTest, commandServiceFailed) {
  NumericGuid sessionId = arlcore::UuidFactory::getInstance().generateGuid();
  GlobalVectorCommandType cmd;
  GlobalVectorCommandStatusType status;
  GlobalVectorCommandAckReportType ack;
  GlobalVectorExecutionStatusReportType report;
  cmd.source().id(consumer_id_.getGuid());
  cmd.sessionID(sessionId.getGuid());
  commandIo_->send(cmd);
  cmdProv->cmdFailed[sessionId] = CommandStatusReasonEnumType::SERVICE_FAILED;
  EXPECT_TRUE(cmdProv->cycle());

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::ISSUED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  EXPECT_EQ(commandAckIo_->read(&ack), ReadStatus::SUCCESS);
  EXPECT_EQ(ack.sessionID(), sessionId.getGuid());
  EXPECT_EQ(ack.command(), cmd);
  EXPECT_EQ(ack.source().id(), provider_id_.getGuid());

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMMANDED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::EXECUTING);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  EXPECT_EQ(cmdExeStatusIo_->read(&report), ReadStatus::SUCCESS);
  EXPECT_EQ(report.sessionID(), sessionId.getGuid());
  EXPECT_EQ(report.source().id(), provider_id_.getGuid());
  EXPECT_FALSE(report.elevationAchieved());
  EXPECT_FALSE(report.speedAchieved());

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::FAILED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SERVICE_FAILED);

  // Check that all status and reports were disposed
  EXPECT_EQ(commandAckIo_->read(&ack), ReadStatus::DISPOSED);
  EXPECT_EQ(ack.sessionID(), sessionId.getGuid());
  EXPECT_EQ(ack.source().id(), provider_id_.getGuid());
  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::DISPOSED);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.source().id(), provider_id_.getGuid());
  EXPECT_EQ(cmdExeStatusIo_->read(&report), ReadStatus::DISPOSED);
  EXPECT_EQ(report.sessionID(), sessionId.getGuid());
  EXPECT_EQ(report.source().id(), provider_id_.getGuid());

  EXPECT_TRUE(cmdProv->cycle());
  EXPECT_EQ(cmdStatusIo_->count(), 0);
}

TEST_F(CommandProviderBaseTest, multipleSessions) {
  NumericGuid sessionId = arlcore::UuidFactory::getInstance().generateGuid();
  GlobalVectorCommandType cmd;
  GlobalVectorCommandStatusType status;
  GlobalVectorCommandAckReportType ack;
  GlobalVectorExecutionStatusReportType report;
  cmd.source().id(consumer_id_.getGuid());
  cmd.sessionID(sessionId.getGuid());
  commandIo_->send(cmd);
  cmdProv->cmdValid[sessionId] = false;
  EXPECT_TRUE(cmdProv->cycle());

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::ISSUED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  EXPECT_EQ(commandAckIo_->read(&ack), ReadStatus::SUCCESS);
  EXPECT_EQ(ack.sessionID(), sessionId.getGuid());
  EXPECT_EQ(ack.command(), cmd);
  EXPECT_EQ(ack.source().id(), provider_id_.getGuid());

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::FAILED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::VALIDATION_FAILED);
  EXPECT_EQ(cmdExeStatusIo_->read(&report), ReadStatus::SUCCESS);
  EXPECT_EQ(report.sessionID(), sessionId.getGuid());
  EXPECT_EQ(report.source().id(), provider_id_.getGuid());
  EXPECT_FALSE(report.elevationAchieved());
  EXPECT_FALSE(report.speedAchieved());

  // Check that all status and reports were disposed
  EXPECT_EQ(commandAckIo_->read(&ack), ReadStatus::DISPOSED);
  EXPECT_EQ(ack.sessionID(), sessionId.getGuid());
  EXPECT_EQ(ack.source().id(), provider_id_.getGuid());
  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::DISPOSED);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.source().id(), provider_id_.getGuid());
  EXPECT_EQ(cmdExeStatusIo_->read(&report), ReadStatus::DISPOSED);
  EXPECT_EQ(report.sessionID(), sessionId.getGuid());
  EXPECT_EQ(report.source().id(), provider_id_.getGuid());

  EXPECT_TRUE(cmdProv->cycle());
  EXPECT_EQ(cmdStatusIo_->count(), 0);

  // New session ID with a "valid" command
  NumericGuid newSessionId = arlcore::UuidFactory::getInstance().generateGuid();
  cmdProv->cmdValid[sessionId] = true;
  cmd.sessionID(newSessionId.getGuid());
  commandIo_->send(cmd);

  EXPECT_TRUE(cmdProv->cycle());
  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), newSessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::ISSUED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  EXPECT_EQ(commandAckIo_->read(&ack), ReadStatus::SUCCESS);
  EXPECT_EQ(ack.sessionID(), newSessionId.getGuid());
  EXPECT_EQ(ack.command(), cmd);
  EXPECT_EQ(ack.source().id(), provider_id_.getGuid());

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), newSessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMMANDED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
}

TEST_F(CommandProviderBaseTest, behaviorCancel) {
  NumericGuid sessionId = arlcore::UuidFactory::getInstance().generateGuid();
  NumericGuid newSessionId = arlcore::UuidFactory::getInstance().generateGuid();
  GlobalVectorCommandType cmd;
  GlobalVectorCommandStatusType status;
  GlobalVectorCommandAckReportType ack;
  GlobalVectorExecutionStatusReportType report;
  cmd.source().id(consumer_id_.getGuid());
  cmd.sessionID(sessionId.getGuid());
  commandIo_->send(cmd);
  EXPECT_TRUE(cmdProv->cycle());

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::ISSUED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  EXPECT_EQ(commandAckIo_->read(&ack), ReadStatus::SUCCESS);
  EXPECT_EQ(ack.sessionID(), sessionId.getGuid());
  EXPECT_EQ(ack.command(), cmd);
  EXPECT_EQ(ack.source().id(), provider_id_.getGuid());

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMMANDED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::EXECUTING);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);

  EXPECT_EQ(cmdStatusIo_->count(), 0);
  cmd.sessionID(newSessionId.getGuid());
  commandIo_->send(cmd);
  EXPECT_TRUE(cmdProv->cycle());
  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::CANCELED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::CANCELED);

  // Check that all status and reports were disposed
  EXPECT_EQ(commandAckIo_->read(&ack), ReadStatus::DISPOSED);
  EXPECT_EQ(ack.sessionID(), sessionId.getGuid());
  EXPECT_EQ(ack.source().id(), provider_id_.getGuid());
  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::DISPOSED);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.source().id(), provider_id_.getGuid());

  // New command session status
  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), newSessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::ISSUED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  EXPECT_EQ(commandAckIo_->read(&ack), ReadStatus::SUCCESS);
  EXPECT_EQ(ack.sessionID(), newSessionId.getGuid());
  EXPECT_EQ(ack.command(), cmd);
  EXPECT_EQ(ack.source().id(), provider_id_.getGuid());

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), newSessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMMANDED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
}

TEST_F(CommandProviderBaseTest, behaviorComplete) {
  NumericGuid sessionId = arlcore::UuidFactory::getInstance().generateGuid();
  NumericGuid newSessionId = arlcore::UuidFactory::getInstance().generateGuid();
  GlobalVectorCommandType cmd;
  GlobalVectorCommandStatusType status;
  GlobalVectorCommandAckReportType ack;
  GlobalVectorExecutionStatusReportType report;

  cmdProv->setBehavior(arlcore::umaa::services::IncomingCommandBehavior::COMPLETE_EXISTING);
  cmd.source().id(consumer_id_.getGuid());
  cmd.sessionID(sessionId.getGuid());
  commandIo_->send(cmd);
  EXPECT_TRUE(cmdProv->cycle());

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::ISSUED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  EXPECT_EQ(commandAckIo_->read(&ack), ReadStatus::SUCCESS);
  EXPECT_EQ(ack.sessionID(), sessionId.getGuid());
  EXPECT_EQ(ack.command(), cmd);
  EXPECT_EQ(ack.source().id(), provider_id_.getGuid());

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMMANDED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::EXECUTING);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);

  EXPECT_EQ(cmdStatusIo_->count(), 0);
  cmd.sessionID(newSessionId.getGuid());
  commandIo_->send(cmd);
  EXPECT_TRUE(cmdProv->cycle());
  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMPLETED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);

  // Check that all status and reports were disposed
  EXPECT_EQ(commandAckIo_->read(&ack), ReadStatus::DISPOSED);
  EXPECT_EQ(ack.sessionID(), sessionId.getGuid());
  EXPECT_EQ(ack.source().id(), provider_id_.getGuid());
  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::DISPOSED);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.source().id(), provider_id_.getGuid());

  // New command session status
  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), newSessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::ISSUED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  EXPECT_EQ(commandAckIo_->read(&ack), ReadStatus::SUCCESS);
  EXPECT_EQ(ack.sessionID(), newSessionId.getGuid());
  EXPECT_EQ(ack.command(), cmd);
  EXPECT_EQ(ack.source().id(), provider_id_.getGuid());

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), newSessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMMANDED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
}

TEST_F(CommandProviderBaseTest, behaviorReject) {
  NumericGuid sessionId = arlcore::UuidFactory::getInstance().generateGuid();
  NumericGuid newSessionId = arlcore::UuidFactory::getInstance().generateGuid();
  GlobalVectorCommandType cmd;
  GlobalVectorCommandStatusType status;
  GlobalVectorCommandAckReportType ack;
  GlobalVectorExecutionStatusReportType report;

  cmdProv->setBehavior(arlcore::umaa::services::IncomingCommandBehavior::REJECT_INCOMING);
  cmd.source().id(consumer_id_.getGuid());
  cmd.sessionID(sessionId.getGuid());
  commandIo_->send(cmd);
  EXPECT_TRUE(cmdProv->cycle());

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::ISSUED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  EXPECT_EQ(commandAckIo_->read(&ack), ReadStatus::SUCCESS);
  EXPECT_EQ(ack.sessionID(), sessionId.getGuid());
  EXPECT_EQ(ack.command(), cmd);
  EXPECT_EQ(ack.source().id(), provider_id_.getGuid());

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMMANDED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::EXECUTING);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);

  EXPECT_EQ(cmdStatusIo_->count(), 0);
  cmd.sessionID(newSessionId.getGuid());
  commandIo_->send(cmd);
  EXPECT_TRUE(cmdProv->cycle());

  // New command session status
  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), newSessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::ISSUED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  EXPECT_EQ(commandAckIo_->read(&ack), ReadStatus::SUCCESS);
  EXPECT_EQ(ack.sessionID(), newSessionId.getGuid());
  EXPECT_EQ(ack.command(), cmd);
  EXPECT_EQ(ack.source().id(), provider_id_.getGuid());
  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), newSessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::FAILED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SERVICE_FAILED);

  // Check that all new command status and reports were disposed
  EXPECT_EQ(commandAckIo_->read(&ack), ReadStatus::DISPOSED);
  EXPECT_EQ(ack.sessionID(), newSessionId.getGuid());
  EXPECT_EQ(ack.source().id(), provider_id_.getGuid());
  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::DISPOSED);
  EXPECT_EQ(status.sessionID(), newSessionId.getGuid());
  EXPECT_EQ(status.source().id(), provider_id_.getGuid());

  // Continue active command session
  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::NO_DATA);
  EXPECT_EQ(cmdExeStatusIo_->read(&report), ReadStatus::SUCCESS);
  EXPECT_EQ(report.sessionID(), sessionId.getGuid());
  EXPECT_EQ(report.source().id(), provider_id_.getGuid());
  EXPECT_FALSE(report.elevationAchieved());
  EXPECT_FALSE(report.speedAchieved());
}

TEST_F(CommandProviderBaseTest, behaviorConcurrent) {
  NumericGuid sessionId = arlcore::UuidFactory::getInstance().generateGuid();
  NumericGuid newSessionId = arlcore::UuidFactory::getInstance().generateGuid();
  GlobalVectorCommandType cmd;
  GlobalVectorCommandStatusType status;
  GlobalVectorCommandAckReportType ack;
  GlobalVectorExecutionStatusReportType report;

  cmdProv->setBehavior(arlcore::umaa::services::IncomingCommandBehavior::ACCEPT_CONCURRENT);
  cmd.source().id(consumer_id_.getGuid());
  cmd.sessionID(sessionId.getGuid());
  commandIo_->send(cmd);
  EXPECT_TRUE(cmdProv->cycle());

  ASSERT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::ISSUED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  ASSERT_EQ(commandAckIo_->read(&ack), ReadStatus::SUCCESS);
  EXPECT_EQ(ack.sessionID(), sessionId.getGuid());
  EXPECT_EQ(ack.command(), cmd);
  EXPECT_EQ(ack.source().id(), provider_id_.getGuid());

  ASSERT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMMANDED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);

  ASSERT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::EXECUTING);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);

  ASSERT_EQ(cmdExeStatusIo_->read(&report), ReadStatus::SUCCESS);
  EXPECT_EQ(report.sessionID(), sessionId.getGuid());
  EXPECT_EQ(report.source().id(), provider_id_.getGuid());
  EXPECT_FALSE(report.elevationAchieved());
  EXPECT_FALSE(report.speedAchieved());

  EXPECT_EQ(cmdStatusIo_->count(), 0);
  cmd.sessionID(newSessionId.getGuid());
  commandIo_->send(cmd);
  EXPECT_TRUE(cmdProv->cycle());

  // New command session status
  ASSERT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), newSessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::ISSUED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  ASSERT_EQ(commandAckIo_->read(&ack), ReadStatus::SUCCESS);
  EXPECT_EQ(ack.sessionID(), newSessionId.getGuid());
  EXPECT_EQ(ack.command(), cmd);
  EXPECT_EQ(ack.source().id(), provider_id_.getGuid());

  ASSERT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), newSessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMMANDED);

  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);

  ASSERT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), newSessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::EXECUTING);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);

  // Validate session execution statuses sent
  ASSERT_EQ(cmdExeStatusIo_->read(&report), ReadStatus::SUCCESS);
  ASSERT_EQ(cmdExeStatusIo_->read(&report), ReadStatus::SUCCESS);

  EXPECT_EQ(cmdProv->getActiveCommands().size(), 2);
  auto state = cmdProv->getCommandState(sessionId);
  ASSERT_TRUE(state.has_value());
  EXPECT_EQ(state->first, CommandStatusEnumType::EXECUTING);
  EXPECT_EQ(state->second, CommandStatusReasonEnumType::SUCCEEDED);

  cmdProv->cmdComplete[sessionId] = true;

  EXPECT_TRUE(cmdProv->cycle());

  ASSERT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);

  // // Continue active command session
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMPLETED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);

  // Validate session execution statuses sent
  ASSERT_EQ(cmdExeStatusIo_->read(&report), ReadStatus::SUCCESS);
  ASSERT_EQ(cmdExeStatusIo_->read(&report), ReadStatus::SUCCESS);

  // Check that original command status and reports were disposed
  ASSERT_EQ(commandAckIo_->read(&ack), ReadStatus::DISPOSED);
  EXPECT_EQ(ack.sessionID(), sessionId.getGuid());
  EXPECT_EQ(ack.source().id(), provider_id_.getGuid());
  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::DISPOSED);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.source().id(), provider_id_.getGuid());
  ASSERT_EQ(cmdExeStatusIo_->read(&report), ReadStatus::DISPOSED);
  EXPECT_EQ(report.sessionID(), sessionId.getGuid());

  EXPECT_TRUE(cmdProv->cycle());

  // Continue new command session
  ASSERT_EQ(cmdExeStatusIo_->read(&report), ReadStatus::SUCCESS);
  EXPECT_EQ(report.sessionID(), newSessionId.getGuid());
  EXPECT_EQ(report.source().id(), provider_id_.getGuid());
  EXPECT_FALSE(report.elevationAchieved());
  EXPECT_FALSE(report.speedAchieved());

  commandIo_->dispose(cmd);
  EXPECT_TRUE(cmdProv->cycle());

  ASSERT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), newSessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::CANCELED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::CANCELED);

  EXPECT_EQ(cmdExeStatusIo_->read(&report), ReadStatus::SUCCESS);
  EXPECT_EQ(report.sessionID(), newSessionId.getGuid());
  EXPECT_EQ(report.source().id(), provider_id_.getGuid());
  EXPECT_FALSE(report.elevationAchieved());
  EXPECT_FALSE(report.speedAchieved());

  ASSERT_EQ(cmdStatusIo_->read(&status), ReadStatus::DISPOSED);
  EXPECT_EQ(status.sessionID(), newSessionId.getGuid());

  ASSERT_EQ(cmdExeStatusIo_->read(&report), ReadStatus::DISPOSED);
  EXPECT_EQ(report.sessionID(), newSessionId.getGuid());
}

TEST_F(CommandProviderBaseTest, behaviorQueueSuccess) {
  NumericGuid sessionId = arlcore::UuidFactory::getInstance().generateGuid();
  NumericGuid newSessionId = arlcore::UuidFactory::getInstance().generateGuid();
  GlobalVectorCommandType cmd;
  GlobalVectorCommandStatusType status;
  GlobalVectorCommandAckReportType ack;
  GlobalVectorExecutionStatusReportType report;

  // TEST SUCCESS

  cmdProv->setBehavior(arlcore::umaa::services::IncomingCommandBehavior::QUEUE_INCOMING);
  cmd.source().id(consumer_id_.getGuid());
  cmd.sessionID(sessionId.getGuid());
  commandIo_->send(cmd);
  EXPECT_TRUE(cmdProv->cycle());

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::ISSUED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  EXPECT_EQ(commandAckIo_->read(&ack), ReadStatus::SUCCESS);
  EXPECT_EQ(ack.sessionID(), sessionId.getGuid());
  EXPECT_EQ(ack.command(), cmd);
  EXPECT_EQ(ack.source().id(), provider_id_.getGuid());

  EXPECT_TRUE(cmdProv->cycle());
  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMMANDED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);

  EXPECT_TRUE(cmdProv->cycle());
  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::EXECUTING);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);

  EXPECT_EQ(cmdStatusIo_->count(), 0);
  cmd.sessionID(newSessionId.getGuid());
  commandIo_->send(cmd);
  EXPECT_TRUE(cmdProv->cycle());

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), newSessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::ISSUED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  EXPECT_EQ(commandAckIo_->read(&ack), ReadStatus::SUCCESS);
  EXPECT_EQ(ack.sessionID(), newSessionId.getGuid());
  EXPECT_EQ(ack.command(), cmd);
  EXPECT_EQ(ack.source().id(), provider_id_.getGuid());

  cmdProv->cmdComplete[sessionId] = true;
  EXPECT_TRUE(cmdProv->cycle());

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMPLETED);

  EXPECT_TRUE(cmdProv->cycle());

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::DISPOSED);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.source().id(), provider_id_.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::CANCELED);

  EXPECT_TRUE(cmdProv->cycle());

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), newSessionId.getGuid());
  EXPECT_EQ(status.source().id(), provider_id_.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMMANDED);

  EXPECT_TRUE(cmdProv->cycle());
  EXPECT_EQ(cmdStatusIo_->readLatest(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), newSessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::EXECUTING);

  cmdProv->cmdComplete[newSessionId] = true;
  EXPECT_TRUE(cmdProv->cycle());

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), newSessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMPLETED);

  EXPECT_TRUE(cmdProv->cycle());

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::DISPOSED);
  EXPECT_EQ(status.sessionID(), newSessionId.getGuid());
  EXPECT_EQ(status.source().id(), provider_id_.getGuid());
}

TEST_F(CommandProviderBaseTest, behaviorQueueFailed) {
  NumericGuid sessionId = arlcore::UuidFactory::getInstance().generateGuid();
  NumericGuid newSessionId = arlcore::UuidFactory::getInstance().generateGuid();
  GlobalVectorCommandType cmd;
  GlobalVectorCommandStatusType status;
  GlobalVectorCommandAckReportType ack;
  GlobalVectorExecutionStatusReportType report;

  // TEST FAIL
  cmdProv->setBehavior(arlcore::umaa::services::IncomingCommandBehavior::QUEUE_INCOMING);
  cmd.source().id(consumer_id_.getGuid());
  cmd.sessionID(sessionId.getGuid());
  commandIo_->send(cmd);
  EXPECT_TRUE(cmdProv->cycle());

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::ISSUED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  EXPECT_EQ(commandAckIo_->read(&ack), ReadStatus::SUCCESS);
  EXPECT_EQ(ack.sessionID(), sessionId.getGuid());
  EXPECT_EQ(ack.command(), cmd);
  EXPECT_EQ(ack.source().id(), provider_id_.getGuid());

  EXPECT_TRUE(cmdProv->cycle());
  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMMANDED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);

  EXPECT_TRUE(cmdProv->cycle());
  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::EXECUTING);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);

  EXPECT_EQ(cmdStatusIo_->count(), 0);
  cmd.sessionID(newSessionId.getGuid());
  commandIo_->send(cmd);
  EXPECT_TRUE(cmdProv->cycle());

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), newSessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::ISSUED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  EXPECT_EQ(commandAckIo_->read(&ack), ReadStatus::SUCCESS);
  EXPECT_EQ(ack.sessionID(), newSessionId.getGuid());
  EXPECT_EQ(ack.command(), cmd);
  EXPECT_EQ(ack.source().id(), provider_id_.getGuid());

  cmdProv->cmdFailed[sessionId] = CommandStatusReasonEnumType::SERVICE_FAILED;
  EXPECT_TRUE(cmdProv->cycle());

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::FAILED);

  EXPECT_TRUE(cmdProv->cycle());

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::DISPOSED);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.source().id(), provider_id_.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::CANCELED);

  EXPECT_TRUE(cmdProv->cycle());

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), newSessionId.getGuid());
  EXPECT_EQ(status.source().id(), provider_id_.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMMANDED);

  EXPECT_TRUE(cmdProv->cycle());
  EXPECT_EQ(cmdStatusIo_->readLatest(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), newSessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::EXECUTING);

  cmdProv->cmdComplete[newSessionId] = true;
  EXPECT_TRUE(cmdProv->cycle());

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), newSessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMPLETED);

  EXPECT_TRUE(cmdProv->cycle());

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::DISPOSED);
  EXPECT_EQ(status.sessionID(), newSessionId.getGuid());
  EXPECT_EQ(status.source().id(), provider_id_.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::CANCELED);
}

TEST_F(CommandProviderBaseTest, behaviorQueueCanceled) {
  NumericGuid sessionId = arlcore::UuidFactory::getInstance().generateGuid();
  NumericGuid newSessionId = arlcore::UuidFactory::getInstance().generateGuid();
  GlobalVectorCommandType cmd;
  GlobalVectorCommandStatusType status;
  GlobalVectorCommandAckReportType ack;
  GlobalVectorExecutionStatusReportType report;

  // TEST CANCEL
  cmdProv->setBehavior(arlcore::umaa::services::IncomingCommandBehavior::QUEUE_INCOMING);
  cmd.source().id(consumer_id_.getGuid());
  cmd.sessionID(sessionId.getGuid());
  commandIo_->send(cmd);
  EXPECT_TRUE(cmdProv->cycle());

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::ISSUED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  EXPECT_EQ(commandAckIo_->read(&ack), ReadStatus::SUCCESS);
  EXPECT_EQ(ack.sessionID(), sessionId.getGuid());
  EXPECT_EQ(ack.command(), cmd);
  EXPECT_EQ(ack.source().id(), provider_id_.getGuid());

  EXPECT_TRUE(cmdProv->cycle());
  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMMANDED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);

  EXPECT_TRUE(cmdProv->cycle());
  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::EXECUTING);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);

  EXPECT_EQ(cmdStatusIo_->count(), 0);
  cmd.sessionID(newSessionId.getGuid());
  commandIo_->send(cmd);
  EXPECT_TRUE(cmdProv->cycle());

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), newSessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::ISSUED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  EXPECT_EQ(commandAckIo_->read(&ack), ReadStatus::SUCCESS);
  EXPECT_EQ(ack.sessionID(), newSessionId.getGuid());
  EXPECT_EQ(ack.command(), cmd);
  EXPECT_EQ(ack.source().id(), provider_id_.getGuid());

  cmd.source().id(consumer_id_.getGuid());
  cmd.sessionID(sessionId.getGuid());
  commandIo_->dispose(cmd);

  EXPECT_TRUE(cmdProv->cycle());
  ASSERT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.source().id(), provider_id_.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::CANCELED);

  ASSERT_EQ(cmdStatusIo_->read(&status), ReadStatus::DISPOSED);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.source().id(), provider_id_.getGuid());

  ASSERT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), newSessionId.getGuid());
  EXPECT_EQ(status.source().id(), provider_id_.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMMANDED);

  EXPECT_TRUE(cmdProv->cycle());
  ASSERT_EQ(cmdStatusIo_->readLatest(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), newSessionId.getGuid());
  EXPECT_EQ(status.source().id(), provider_id_.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::EXECUTING);
}

TEST_F(CommandProviderBaseTest, behaviorQueueCancelQueued) {
  NumericGuid sessionId = arlcore::UuidFactory::getInstance().generateGuid();
  NumericGuid newSessionId = arlcore::UuidFactory::getInstance().generateGuid();
  GlobalVectorCommandType cmd;
  GlobalVectorCommandStatusType status;
  GlobalVectorCommandAckReportType ack;
  GlobalVectorExecutionStatusReportType report;

  cmdProv->setBehavior(arlcore::umaa::services::IncomingCommandBehavior::QUEUE_INCOMING);
  cmd.source().id(consumer_id_.getGuid());
  cmd.sessionID(sessionId.getGuid());
  commandIo_->send(cmd);
  EXPECT_TRUE(cmdProv->cycle());

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::ISSUED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  EXPECT_EQ(commandAckIo_->read(&ack), ReadStatus::SUCCESS);
  EXPECT_EQ(ack.sessionID(), sessionId.getGuid());
  EXPECT_EQ(ack.command(), cmd);
  EXPECT_EQ(ack.source().id(), provider_id_.getGuid());

  EXPECT_TRUE(cmdProv->cycle());
  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMMANDED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);

  EXPECT_TRUE(cmdProv->cycle());
  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::EXECUTING);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);

  EXPECT_EQ(cmdStatusIo_->count(), 0);
  cmd.sessionID(newSessionId.getGuid());
  commandIo_->send(cmd);
  EXPECT_TRUE(cmdProv->cycle());

  EXPECT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), newSessionId.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::ISSUED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  EXPECT_EQ(commandAckIo_->read(&ack), ReadStatus::SUCCESS);
  EXPECT_EQ(ack.sessionID(), newSessionId.getGuid());
  EXPECT_EQ(ack.command(), cmd);
  EXPECT_EQ(ack.source().id(), provider_id_.getGuid());

  commandIo_->dispose(cmd);

  EXPECT_TRUE(cmdProv->cycle());

  ASSERT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), newSessionId.getGuid());
  EXPECT_EQ(status.source().id(), provider_id_.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::CANCELED);

  ASSERT_EQ(cmdStatusIo_->read(&status), ReadStatus::DISPOSED);
  EXPECT_EQ(status.sessionID(), newSessionId.getGuid());
  EXPECT_EQ(status.source().id(), provider_id_.getGuid());

  cmdProv->cmdComplete[sessionId] = true;
  EXPECT_TRUE(cmdProv->cycle());

  ASSERT_EQ(cmdStatusIo_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.source().id(), provider_id_.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMPLETED);

  EXPECT_TRUE(cmdProv->cycle());
  ASSERT_EQ(cmdStatusIo_->read(&status), ReadStatus::DISPOSED);
  EXPECT_EQ(status.sessionID(), sessionId.getGuid());
  EXPECT_EQ(status.source().id(), provider_id_.getGuid());
}
