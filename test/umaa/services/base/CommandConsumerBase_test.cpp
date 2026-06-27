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

#include "CommandConsumerBase.h"

#include <gtest/gtest.h>

#include <memory>

#include <UMAA/Common/Measurement/ElevationVariantType.hpp>
#include <UMAA/MO/GlobalVectorControl/GlobalVectorCommandAckReportType.hpp>
#include <UMAA/MO/GlobalVectorControl/GlobalVectorCommandStatusType.hpp>
#include <UMAA/MO/GlobalVectorControl/GlobalVectorCommandType.hpp>
#include <UMAA/MO/GlobalVectorControl/GlobalVectorExecutionStatusReportType.hpp>
#include <dds/dds.hpp>

#include "LocalReaderSender.h"
#include "Logger.h"
#include "NumericGuid.h"

using VectorCmdType = UMAA::MO::GlobalVectorControl::GlobalVectorCommandType;
using VectorCmdAckType = UMAA::MO::GlobalVectorControl::GlobalVectorCommandAckReportType;
using VectorCmdStatusType = UMAA::MO::GlobalVectorControl::GlobalVectorCommandStatusType;
using VectorCmdExeStatusType = UMAA::MO::GlobalVectorControl::GlobalVectorExecutionStatusReportType;
using ElevationRequirementVariantTypeEnum = UMAA::Common::Measurement::ElevationRequirementVariantTypeEnum;

using GlobalVectorCommandIo = arlcore::umaa::domain::
    UmaaCommandConsumerIo<VectorCmdType, VectorCmdAckType, VectorCmdStatusType, VectorCmdExeStatusType>;

using GlobalVectorCommandConsumer = arlcore::umaa::services::
    CommandConsumerBase<VectorCmdType, VectorCmdAckType, VectorCmdStatusType, VectorCmdExeStatusType>;

using UmaaCommandHeader = arlcore::umaa::domain::CommandHeader;
using arlcore::io::ReadStatus;
using arlcore::io::SendStatus;

class CommandConsumerBaseTest : public ::testing::Test {
 protected:
  CommandConsumerBaseTest() {
    // CONSUMER
    commandHeader_.sourceId.id() = {68, 101, 118, 111, 110, 39, 115, 32, 67, 111, 110, 115, 117, 109, 101, 114};
    commandHeader_.destinationId.id() = {70, 97, 107, 101, 86, 101, 99, 116, 111, 114, 80, 114, 111, 118, 0, 0};

    auto io = std::make_shared<GlobalVectorCommandIo>(
        cmdSender_ = std::make_shared<arlcore::io::LocalReaderSender<VectorCmdType>>(),
        ackReader_ = std::make_shared<arlcore::io::LocalReaderSender<VectorCmdAckType>>(),
        statReader_ = std::make_shared<arlcore::io::LocalReaderSender<VectorCmdStatusType>>(),
        exeStatReader_ = std::make_shared<arlcore::io::LocalReaderSender<VectorCmdExeStatusType>>());

    gvConsumer_ = std::make_unique<GlobalVectorCommandConsumer>(commandHeader_, io);
  }

  VectorCmdType getVectorCommand() {
    VectorCmdType cmd;

    // Elevation is an optional so must be set like this...
    UMAA::Common::Measurement::ElevationRequirementVariantType depthVariant;
    depthVariant.ElevationRequirementVariantTypeSubtypes().DepthRequirementVariantVariant(
        UMAA::Common::Measurement::DepthRequirementVariantType());
    depthVariant.ElevationRequirementVariantTypeSubtypes().DepthRequirementVariantVariant().depth().depth(25);
    cmd.elevation(depthVariant);

    return cmd;
  }

  UmaaCommandHeader commandHeader_;
  std::unique_ptr<GlobalVectorCommandConsumer> gvConsumer_;
  std::shared_ptr<arlcore::io::LocalReaderSender<VectorCmdType>> cmdSender_;
  std::shared_ptr<arlcore::io::LocalReaderSender<VectorCmdAckType>> ackReader_;
  std::shared_ptr<arlcore::io::LocalReaderSender<VectorCmdStatusType>> statReader_;
  std::shared_ptr<arlcore::io::LocalReaderSender<VectorCmdExeStatusType>> exeStatReader_;
};

// Example of specializing isValidAck
template <>
template <>
bool GlobalVectorCommandConsumer::isValidAck<VectorCmdAckType>(const VectorCmdAckType& ack,
                                                               const arlcore::umaa::domain::CommandHeader& header) {
  return ack.source().id() == header.destinationId.id() && ack.sessionID() == header.sessionId.getGuid();
}

TEST_F(CommandConsumerBaseTest, IsAckValid) {
  NumericGuid sessionId = arlcore::UuidFactory::getInstance().generateGuid();
  NumericGuid destId = arlcore::UuidFactory::getInstance().generateGuid();
  VectorCmdAckType ack;
  arlcore::umaa::domain::CommandHeader header;
  ack.source().id(destId.getGuid());
  ack.sessionID(sessionId.getGuid());
  header.destinationId.id(destId.getGuid());
  header.sessionId = sessionId;
  EXPECT_TRUE(GlobalVectorCommandConsumer::isValidAck(ack, header));
  ack.sessionID(arlcore::NIL_GUID.getGuid());
  EXPECT_FALSE(GlobalVectorCommandConsumer::isValidAck(ack, header));
  ack.sessionID(sessionId.getGuid());
  ack.source().id(arlcore::NIL_GUID.getGuid());
  EXPECT_FALSE(GlobalVectorCommandConsumer::isValidAck(ack, header));
}

TEST_F(CommandConsumerBaseTest, IsTestValid) {
  NumericGuid sessionId = arlcore::UuidFactory::getInstance().generateGuid();
  NumericGuid destId = arlcore::UuidFactory::getInstance().generateGuid();
  VectorCmdStatusType status;
  arlcore::umaa::domain::CommandHeader header;
  status.source().id(destId.getGuid());
  status.sessionID(sessionId.getGuid());
  header.destinationId.id(destId.getGuid());
  header.sessionId = sessionId;
  EXPECT_TRUE(GlobalVectorCommandConsumer::isValidStatus(status, header));
  status.sessionID(arlcore::NIL_GUID.getGuid());
  EXPECT_FALSE(GlobalVectorCommandConsumer::isValidStatus(status, header));
  status.sessionID(sessionId.getGuid());
  status.source().id(arlcore::NIL_GUID.getGuid());
  EXPECT_FALSE(GlobalVectorCommandConsumer::isValidStatus(status, header));
}

TEST_F(CommandConsumerBaseTest, OpenCloseCommandSession) {
  EXPECT_EQ(arlcore::NIL_GUID, gvConsumer_->getSessionId());
  EXPECT_FALSE(gvConsumer_->isCommandSessionOpen());
  EXPECT_TRUE(gvConsumer_->openCommandSession());
  EXPECT_FALSE(gvConsumer_->openCommandSession());
  EXPECT_TRUE(gvConsumer_->isCommandSessionOpen());
  EXPECT_NE(arlcore::NIL_GUID, gvConsumer_->getSessionId());
  EXPECT_EQ(SendStatus::SUCCESS, gvConsumer_->closeCommandSession());
  EXPECT_EQ(SendStatus::ERROR, gvConsumer_->closeCommandSession());
}

TEST_F(CommandConsumerBaseTest, SendCommand) {
  VectorCmdType cmd = getVectorCommand();
  EXPECT_EQ(SendStatus::SUCCESS, gvConsumer_->send(&cmd));

  ASSERT_EQ(cmdSender_->count(), 1);
  VectorCmdType recvCmd;
  ASSERT_EQ(cmdSender_->read(&recvCmd), ReadStatus::SUCCESS);
  EXPECT_EQ(cmd, recvCmd);
  EXPECT_EQ(SendStatus::SUCCESS, gvConsumer_->closeCommandSession());
}

TEST_F(CommandConsumerBaseTest, ReadAck) {
  VectorCmdAckType recvAck;
  ASSERT_EQ(ReadStatus::INVALID_DATA, gvConsumer_->read(&recvAck));

  VectorCmdType cmd = getVectorCommand();
  EXPECT_EQ(SendStatus::SUCCESS, gvConsumer_->send(&cmd));

  VectorCmdAckType cmdAckValidSource;
  VectorCmdAckType cmdAckInvalidSource;
  cmdAckValidSource.source().id(commandHeader_.destinationId.id());
  cmdAckValidSource.sessionID(gvConsumer_->getSessionId().getGuid());
  cmdAckInvalidSource.source().id({67, 111, 108, 105, 110, 32, 105, 115, 110, 39, 116, 32, 99, 111, 111, 108});
  cmdAckInvalidSource.sessionID(gvConsumer_->getSessionId().getGuid());
  ackReader_->send(cmdAckValidSource);
  ASSERT_EQ(ReadStatus::SUCCESS, gvConsumer_->read(&recvAck));
  EXPECT_EQ(recvAck, cmdAckValidSource);
  ackReader_->send(cmdAckInvalidSource);
  ASSERT_EQ(ReadStatus::NO_DATA, gvConsumer_->read(&recvAck));
  EXPECT_EQ(SendStatus::SUCCESS, gvConsumer_->closeCommandSession());
}

TEST_F(CommandConsumerBaseTest, ReadStatus) {
  VectorCmdStatusType recvStatus;
  ASSERT_EQ(ReadStatus::INVALID_DATA, gvConsumer_->read(&recvStatus));

  VectorCmdType cmd = getVectorCommand();
  EXPECT_EQ(SendStatus::SUCCESS, gvConsumer_->send(&cmd));

  VectorCmdStatusType cmdStatusValidSource;
  VectorCmdStatusType cmdStatusInvalidSource;
  cmdStatusValidSource.source().id(commandHeader_.destinationId.id());
  cmdStatusValidSource.sessionID(gvConsumer_->getSessionId().getGuid());
  cmdStatusValidSource.commandStatus(CommandStatusEnumType::COMPLETED);
  cmdStatusInvalidSource.source().id({67, 111, 108, 105, 110, 32, 105, 115, 110, 39, 116, 32, 99, 111, 111, 108});
  cmdStatusInvalidSource.sessionID(gvConsumer_->getSessionId().getGuid());
  cmdStatusInvalidSource.commandStatus(CommandStatusEnumType::COMPLETED);

  statReader_->send(cmdStatusValidSource);
  ASSERT_EQ(ReadStatus::SUCCESS, gvConsumer_->read(&recvStatus));
  EXPECT_EQ(recvStatus, cmdStatusValidSource);
  EXPECT_FALSE(gvConsumer_->isCommandSessionOpen());

  EXPECT_EQ(SendStatus::SUCCESS, gvConsumer_->send(&cmd));
  statReader_->send(cmdStatusInvalidSource);
  ASSERT_EQ(ReadStatus::NO_DATA, gvConsumer_->read(&recvStatus));
  EXPECT_TRUE(gvConsumer_->isCommandSessionOpen());
}

TEST_F(CommandConsumerBaseTest, ReadExeStatus) {
  VectorCmdExeStatusType recvExeStatus;
  ASSERT_EQ(ReadStatus::INVALID_DATA, gvConsumer_->read(&recvExeStatus));

  VectorCmdType cmd = getVectorCommand();
  EXPECT_EQ(SendStatus::SUCCESS, gvConsumer_->send(&cmd));

  VectorCmdExeStatusType cmdExeStatusValidSource;
  VectorCmdExeStatusType cmdExeStatusInvalidSource;
  cmdExeStatusValidSource.source().id(commandHeader_.destinationId.id());
  cmdExeStatusValidSource.sessionID(gvConsumer_->getSessionId().getGuid());
  cmdExeStatusInvalidSource.source().id({67, 111, 108, 105, 110, 32, 105, 115, 110, 39, 116, 32, 99, 111, 111, 108});
  cmdExeStatusInvalidSource.sessionID(gvConsumer_->getSessionId().getGuid());
  exeStatReader_->send(cmdExeStatusValidSource);
  ASSERT_EQ(ReadStatus::SUCCESS, gvConsumer_->read(&recvExeStatus));
  EXPECT_EQ(recvExeStatus, cmdExeStatusValidSource);
  exeStatReader_->send(cmdExeStatusInvalidSource);
  ASSERT_EQ(ReadStatus::NO_DATA, gvConsumer_->read(&recvExeStatus));
  EXPECT_EQ(SendStatus::SUCCESS, gvConsumer_->closeCommandSession());
}

TEST_F(CommandConsumerBaseTest, ExeStatusNotImplemented) {
  auto io =
      std::make_shared<GlobalVectorCommandIo>(std::make_shared<arlcore::io::LocalReaderSender<VectorCmdType>>(),
                                              std::make_shared<arlcore::io::LocalReaderSender<VectorCmdAckType>>(),
                                              std::make_shared<arlcore::io::LocalReaderSender<VectorCmdStatusType>>());

  GlobalVectorCommandConsumer consumer(commandHeader_, io);
  VectorCmdExeStatusType recvExeStatus;
  ASSERT_EQ(ReadStatus::INVALID_DATA, consumer.read(&recvExeStatus));

  VectorCmdType cmd = getVectorCommand();
  EXPECT_EQ(SendStatus::SUCCESS, consumer.send(&cmd));

  ASSERT_EQ(ReadStatus::NOT_IMPLEMENTED, consumer.read(&recvExeStatus));
  EXPECT_EQ(SendStatus::SUCCESS, consumer.closeCommandSession());
}

TEST_F(CommandConsumerBaseTest, CancelCommand) {
  EXPECT_TRUE(gvConsumer_->openCommandSession());
  VectorCmdType cmd = getVectorCommand();
  NumericGuid sessId = gvConsumer_->getSessionId();

  cmdSender_->clear();  // Clear command reader
  EXPECT_EQ(SendStatus::SUCCESS, gvConsumer_->send(&cmd));
  ASSERT_EQ(cmdSender_->count(), 1);
  VectorCmdType recvCmd;
  ASSERT_EQ(cmdSender_->read(&recvCmd), ReadStatus::SUCCESS);
  EXPECT_EQ(cmd, recvCmd);
  ASSERT_EQ(SendStatus::SUCCESS, gvConsumer_->closeCommandSession());
  ASSERT_EQ(cmdSender_->count(), 1);
  VectorCmdType keySample;
  ASSERT_EQ(cmdSender_->read(&keySample), ReadStatus::DISPOSED);
  EXPECT_EQ(commandHeader_.sourceId.id(), keySample.source().id());
  EXPECT_EQ(commandHeader_.destinationId.id(), keySample.destination().id());
  EXPECT_EQ(sessId.getGuid(), keySample.sessionID());
}

TEST_F(CommandConsumerBaseTest, UpdateCommandHeader) {
  EXPECT_EQ(gvConsumer_->getCmdHeader(), commandHeader_);

  CommandHeader newCmdHdr = {
      {UMAA::Common::IdentifierType(arlcore::UuidFactory::getInstance().generateGuid().getGuid(),
                                    arlcore::UuidFactory::getInstance().generateGuid().getGuid())},
      {UMAA::Common::IdentifierType(arlcore::UuidFactory::getInstance().generateGuid().getGuid(),
                                    arlcore::UuidFactory::getInstance().generateGuid().getGuid())},
      arlcore::UuidFactory::getInstance().generateGuid()};

  UMAA::Common::IdentifierType newDestination{arlcore::UuidFactory::getInstance().generateGuid().getGuid(),
                                              arlcore::UuidFactory::getInstance().generateGuid().getGuid()};

  // Test that trying to update a command with an open command session will fail
  ASSERT_TRUE(gvConsumer_->openCommandSession());
  EXPECT_FALSE(gvConsumer_->setCmdHeader(newCmdHdr));
  EXPECT_FALSE(gvConsumer_->setCmdDestination(newDestination));

  // Close command session and expect the update to succeed
  ASSERT_EQ(gvConsumer_->closeCommandSession(), SendStatus::SUCCESS);
  EXPECT_TRUE(gvConsumer_->setCmdHeader(newCmdHdr));
  EXPECT_EQ(gvConsumer_->getCmdHeader(), newCmdHdr);

  // Set just the destination ID
  EXPECT_TRUE(gvConsumer_->setCmdDestination(newDestination));
  EXPECT_NE(gvConsumer_->getCmdHeader(), newCmdHdr);
  EXPECT_EQ(gvConsumer_->getCmdHeader().destinationId, newDestination);
}
