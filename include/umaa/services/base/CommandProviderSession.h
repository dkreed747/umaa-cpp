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

#ifndef INCLUDE_UMAA_SERVICES_BASE_COMMANDPROVIDERSESSION_H_
#define INCLUDE_UMAA_SERVICES_BASE_COMMANDPROVIDERSESSION_H_

#include <string>
#include <memory>

#include "CommandStateMachine.h"
#include "UmaaCommandProviderIo.h"
#include "UmaaUtils.h"

using arlcore::umaa::domain::UmaaCommandProviderIo;
using arlcore::io::SendStatus;

namespace arlcore::umaa::services {

//! \brief A class representing a UMAA command and its session information
//! \tparam CmdType The type of the command
//! \tparam CmdAck The type of the command acknowledgment
//! \tparam CmdStatus The type of the command status
//! \tparam CmdExeStatus The type of the command execution status (nullptr by default)
template <class CmdType, class CmdAck, class CmdStatus, class CmdExeStatus = std::nullptr_t>
class CommandProviderSession {
 public:
  //! \brief Constructor
  //! \param source The source id
  //! \param command The command associated with the session
  //! \param io The IO object for the command provider
  //! \param sendExecutionStatusCallback A callback for sending an execution status message
  //! \param disposeExecutionStatusCallback A callback for disposing an execution status message
  CommandProviderSession(const NumericGuid& source, CmdType command,
    std::shared_ptr<UmaaCommandProviderIo<CmdType, CmdAck, CmdStatus, CmdExeStatus>> io,
    std::function<SendStatus(CmdType)> sendExecutionStatusCallback,
    std::function<SendStatus(CmdType)> disposeExecutionStatusCallback) :
      source_(source),
      command_(command),
      id_(NumericGuid(command.sessionID())),
      io_(io),
      sendExecutionStatusCallback_(sendExecutionStatusCallback),
      disposeExecutionStatusCallback_(disposeExecutionStatusCallback) {}

  CommandProviderSession(CommandProviderSession& other) = delete;  // NOLINT
  CommandProviderSession(CommandProviderSession &&other) noexcept = delete;
  CommandProviderSession& operator=(const CommandProviderSession& other) = delete;
  CommandProviderSession const & operator=(CommandProviderSession &&other) = delete;

  //! \brief Destructor
  ~CommandProviderSession() {
    disposeAck();
    disposeStatus();
    if (disposeExeStatus_) {
      disposeExecutionStatus();
    }
  }
  //! \brief Get the command status of the command corresponding to this session
  //! \return Command status
  CommandStatusEnumType getCommandStatus() const {
    return state_.getState();
  }
  //! \brief Get the command status reason of the command corresponding to this session
  //! \return Command status reason
  CommandStatusReasonEnumType getCommandStatusReason() const {
    return state_.getReason();
  }
  //! \brief Send an acknowledgment for the command corresponding to this session
  //! \return The result of sending the Ack
  SendStatus sendAck() {
    CmdAck ackToSend;
    ackToSend.source().id(source_.getGuid());
    ackToSend.sessionID(id_.getGuid());
    ackToSend.timeStamp(arlcore::umaa::getTimestamp());
    ackToSend.command(command_);

    return io_->cmdAckSender->send(ackToSend);
  }
  //! \brief Send a status message with the current status of this session's command
  //! \param logMessage An optional log message to include with the status
  //! \return The result of sending the status
  SendStatus sendStatus(const std::string& logMessage = "") {
    return sendStatus(state_.getState(), state_.getReason(), logMessage);
  }
  //! \brief Send a status message for the current session with the provided status information
  //! \param status The command status to send in the status message
  //! \param reason The command status reason to send in the status message
  //! \param logMessage An optional log message to include with the status
  //! \return The result of sending the status
  SendStatus sendStatus(CommandStatusEnumType status, CommandStatusReasonEnumType reason,
      const std::string& logMessage = "") {
    CmdStatus statusToSend;
    statusToSend.source().id(source_.getGuid());
    statusToSend.sessionID(id_.getGuid());
    statusToSend.commandStatus(status);
    statusToSend.commandStatusReason(reason);
    statusToSend.logMessage(logMessage);
    statusToSend.timeStamp(arlcore::umaa::getTimestamp());

    return io_->cmdStatusSender->send(statusToSend);
  }
  //! \brief Send an execution status message for the session's command
  //! \return The result of sending the execution status
  SendStatus sendExecutionStatus() {
    SendStatus status = sendExecutionStatusCallback_(command_);
    if (status == SendStatus::SUCCESS) {
      disposeExeStatus_ = true;
    }
    return status;
  }
  //! \brief Advance the state of the session's command
  //! \return Whether the command's state could successfully be advanced
  bool advanceState() {
    return state_.advanceState();
  }
  //! \brief Update the session's command
  //! \param incoming The new command for the session
  //! \return Whether the session's command could be updated successfully
  bool update(CmdType incoming) {
    if (incoming.sessionID() != id_.getGuid() || !state_.update()) {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Unable to update the command for session ID " << id_)
      return false;
    }
    command_ = incoming;
    return true;
  }
  //! \brief Cancel the command associated with this session
  //! \return Whether the command could be canceled
  bool cancel() {
    return state_.cancel();
  }
  //! \brief Get the command associated with this session
  //! \return The session's command
  CmdType getCommand() {
    return command_;
  }
  //! \brief Fail the command associated with this session for the provided reason
  //! \param reason The reason for the command failure
  //! \return Whether the command could be failed
  bool fail(CommandStatusReasonEnumType reason) {
    return state_.fail(reason);
  }
  //! \brief Return the ID of this session
  //! \return The session ID
  NumericGuid getSessionId() const {
    return id_;
  }

 private:
  NumericGuid source_;
  NumericGuid id_;
  CommandStateMachine state_;
  CmdType command_;
  std::shared_ptr<UmaaCommandProviderIo<CmdType, CmdAck, CmdStatus, CmdExeStatus>> io_;
  std::function<SendStatus(CmdType)> sendExecutionStatusCallback_;
  std::function<SendStatus(CmdType)> disposeExecutionStatusCallback_;
  bool disposeExeStatus_ = false;

  //! \brief Dispose the Ack for this session
  void disposeAck() {
    CmdAck ackToDispose;
    ackToDispose.source().id(source_.getGuid());
    ackToDispose.sessionID(id_.getGuid());
    ackToDispose.timeStamp(arlcore::umaa::getTimestamp());

    if (io_->cmdAckSender->dispose(ackToDispose) != SendStatus::SUCCESS) {
      UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Unable to dispose command ack for session id: " << id_)
    }
  }
  //! \brief Dispose the status message for this session
  void disposeStatus() {
    CmdStatus statusToDispose;
    statusToDispose.source().id(source_.getGuid());
    statusToDispose.sessionID(id_.getGuid());
    statusToDispose.timeStamp(arlcore::umaa::getTimestamp());

    if (io_->cmdStatusSender->dispose(statusToDispose) != SendStatus::SUCCESS) {
      UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Unable to dispose command status for session id: " << id_)
    }
  }
  //! \brief Dispose the execution status for this session
  void disposeExecutionStatus() {
    if (disposeExecutionStatusCallback_(command_) == SendStatus::ERROR) {
      UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Unable to dispose command execution status for session id: " << id_)
    }
  }
};

}  // namespace arlcore::umaa::services
#endif  // INCLUDE_UMAA_SERVICES_BASE_COMMANDPROVIDERSESSION_H_
