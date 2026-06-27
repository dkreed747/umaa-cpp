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

#ifndef INCLUDE_UMAA_SERVICES_BASE_COMMANDCONSUMERBASE_H_
#define INCLUDE_UMAA_SERVICES_BASE_COMMANDCONSUMERBASE_H_

#include <atomic>
#include <memory>

#include "CommandStateMachine.h"
#include "Logger.h"
#include "UmaaCommand.h"
#include "UmaaCommandConsumerIo.h"
#include "UmaaUtils.h"
#include "UuidFactory.h"

using arlcore::NumericGuid;
using arlcore::io::ReaderBase;
using arlcore::io::ReadStatus;
using arlcore::io::SenderBase;
using arlcore::io::SendStatus;
using arlcore::umaa::domain::CommandHeader;
using arlcore::umaa::domain::UmaaCommandConsumerIo;
using UMAA::Common::MaritimeEnumeration::CommandStatusEnumModule::CommandStatusEnumType;

namespace arlcore::umaa::services {

//! \brief A base class that provides the methods used by all UMAA command service consumer
//! \tparam CmdType the type of the Command
//! \tparam CmdAck the type of the Command Acknowledgement
//! \tparam CmdStatus the type of the Command Status
//! \tparam CmdExeStatus the type of the Command Execution Status if one exists (Defaults to nullptr)
template <class CmdType, class CmdAck, class CmdStatus, class CmdExeStatus = std::nullptr_t>
class CommandConsumerBase {
 public:
  //! \brief Base Constructor
  //! \param sourceId The source ID to use for all messages send by this consumer
  //! \param io Command service consumer IO containing readers and writers needed for the service
  CommandConsumerBase(CommandHeader header,
                      std::shared_ptr<UmaaCommandConsumerIo<CmdType, CmdAck, CmdStatus, CmdExeStatus>> io)
      : header_(header), io_(io) {}

  //! \brief Overridable destructor that cleans up resources
  virtual ~CommandConsumerBase() {
    if (isSessionOpen_) {
      UMAA_LOG_DEBUG(util::SYSTEM_LOGGER, "Closing Command session on shutdown")
      closeCommandSession();
    }
  }

  //! \brief Virtual function for adding extra logic every time a session is opened
  //! \return SendStatus enum
  virtual SendStatus sessionSetUp() {
    UMAA_LOG_TRACE(util::SYSTEM_LOGGER, "Virtual base method called")
    return SendStatus::NOT_IMPLEMENTED;
  }

  //! \brief Virtual function for adding extra logic every time a session is closed
  //! \return SendStatus enum
  virtual SendStatus sessionTearDown() {
    UMAA_LOG_TRACE(util::SYSTEM_LOGGER, "Virtual base method called")
    return SendStatus::NOT_IMPLEMENTED;
  }

  //! \brief Opens a session if necessary and sends a command.
  //! The sent command will be an update if there is currently an active/non-terminal command.
  //! Can be overridden to provide additional logic for large sets and lists
  //! \param cmd the command to send
  //! \return SendStatus enum
  virtual SendStatus send(CmdType* cmd) {
    if (cmd == nullptr) {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "cmd pointer is null.")
      return SendStatus::ERROR;
    }

    // Check if command update can/should be sent
    if (isCommandSessionOpen()) {
      if (hasCommandTerminated())
        UMAA_LOG_WARN(util::SYSTEM_LOGGER,
                      "Cannot send update for a command that has terminated. Opening new session...")
      else
        UMAA_LOG_DEBUG(
            util::SYSTEM_LOGGER,
            "There is already an active command, sending new command as update for session " << header_.sessionId)
    }

    // Try to open command session if not already open
    if (!isCommandSessionOpen() && !openCommandSession()) return SendStatus::ERROR;

    // Data members on UMAA::UMAACommand - if a non-command type is passed into the template, the code will not compile
    cmd->source().id(header_.sourceId.id());
    cmd->destination().id(header_.destinationId.id());
    cmd->sessionID(header_.sessionId.getGuid());
    cmd->timeStamp(arlcore::umaa::getTimestamp());

    SendStatus status = io_->cmdSender->send(*cmd);
    if (status == SendStatus::SUCCESS) isCommandAlive_ = true;

    return status;
  }

  //! \brief Default function used to determine if an Ack is valid for this consumer. Can be specialized to perform
  //!        additional checks without causing compiler errors if fields don't exist.
  //! \tparam T Type of Ack. Defaults to CmdAck. Used for specialization.
  //! \param ack The Ack object to validate
  //! \param header The header to validate the Ack against
  //! \return Whether the provided Ack is intended for the current consumer
  template <class T = CmdAck>
  static bool isValidAck(const T& ack, const CommandHeader& header) {
    return ack.source().id() == header.destinationId.id() && ack.sessionID() == header.sessionId.getGuid();
  }

  //! \brief Reads the next command ack off the bus
  //! \param outCmdAck pointer to where to write the ack
  //! \return ReadStatus enum
  virtual ReadStatus read(CmdAck* outCmdAck) {
    if (outCmdAck == nullptr) {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "outCmdAck pointer is null.")
      return ReadStatus::ERROR;
    }

    if (!isCommandAlive_) {
      UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Cannot read a command acknowledgment sample without an active command")
      return ReadStatus::INVALID_DATA;
    }

    ReadStatus outStatus;
    bool validData = false;
    do {
      CmdAck tempCmdAck;
      outStatus = io_->cmdAckReader->read(&tempCmdAck);
      switch (outStatus) {
        case ReadStatus::SUCCESS:
        case ReadStatus::DISPOSED:
          if (!isValidAck(tempCmdAck, header_)) break;

          validData = true;
          *outCmdAck = tempCmdAck;
          break;
      }
    } while (!validData && outStatus != ReadStatus::NO_DATA);

    return outStatus;
  }

  //! \brief Default function used to determine if a command status is valid for this consumer. Can be specialized to
  //!        perform additional checks without causing compiler errors if fields don't exist.
  //! \tparam T Type of command status. Defaults to CmdStatus. Used for specialization.
  //! \param status The command status object to validate
  //! \param header The header to validate the command status against
  //! \return Whether the provided command status is intended for the current consumer
  template <class T = CmdStatus>
  static bool isValidStatus(const T& status, const CommandHeader& header) {
    return status.source().id() == header.destinationId.id() && status.sessionID() == header.sessionId.getGuid();
  }

  //! \brief Reads the next command status off the bus.
  //! If a terminal status is read, then the session will be closed if open.
  //! \param outCmdStatus pointer to where to write the command status
  //! \return ReadStatus enum
  virtual ReadStatus read(CmdStatus* outCmdStatus) {
    if (outCmdStatus == nullptr) {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "outCmdStatus pointer is null!")
      return ReadStatus::ERROR;
    }

    if (!isCommandAlive_) {
      UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Cannot read a command status sample without an active command")
      return ReadStatus::INVALID_DATA;
    }

    ReadStatus outStatus;
    bool validData = false;
    do {
      CmdStatus tempCmdStatus;
      outStatus = io_->cmdStatusReader->read(&tempCmdStatus);
      switch (outStatus) {
        case ReadStatus::SUCCESS:
        case ReadStatus::DISPOSED:
          if (!isValidStatus(tempCmdStatus, header_)) break;

          validData = true;
          *outCmdStatus = tempCmdStatus;

          if (!arlcore::umaa::CommandStateMachine::isStateFinal(tempCmdStatus.commandStatus())) break;

          isCommandAlive_ = false;
          if (isCommandSessionOpen()) {
            UMAA_LOG_DEBUG(util::SYSTEM_LOGGER, "Closing command session due to reading a terminal status")
            if (closeCommandSession() != SendStatus::SUCCESS) {
              return ReadStatus::ERROR;
            }
          }
          break;
      }
    } while (!validData && outStatus != ReadStatus::NO_DATA);

    return outStatus;
  }

  //! \brief Default function used to determine if a command execution status is valid for this consumer. Can be
  //! specialized to perform additional checks without causing compiler errors if fields don't exist.
  //! \tparam T Type of command execution status. Defaults to CmdExeStatus. Used for specialization.
  //! \param status The command execution status object to validate
  //! \param header The header to validate the command execution status against
  //! \return Whether the provided command execution status is intended for the current consumer
  template <class T = CmdExeStatus>
  static bool isValidExeStatus(const T& status, const CommandHeader& header) {
    return status.source().id() == header.destinationId.id() && status.sessionID() == header.sessionId.getGuid();
  }

  //! \brief Reads the next command execution status off the bus
  //! \param outCmdExeStatus pointer to where to write the command execution status
  //! \return ReadStatus enum
  virtual ReadStatus read(CmdExeStatus* outCmdExeStatus) {
    if (outCmdExeStatus == nullptr) {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "outCmdExeStatus pointer is null.")
      return ReadStatus::ERROR;
    }

    if (!isCommandAlive_) {
      UMAA_LOG_DEBUG(util::SYSTEM_LOGGER, "Cannot read a command execution status sample without an active command")
      return ReadStatus::INVALID_DATA;
    }

    if (!io_->cmdExeStatusReader.has_value()) {
      UMAA_LOG_DEBUG(util::SYSTEM_LOGGER, "No command execution status reader provided")
      return ReadStatus::NOT_IMPLEMENTED;
    }

    ReadStatus outStatus;
    if constexpr (std::is_same_v<CmdExeStatus, std::nullptr_t>) {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "No command execution status type specified for consumer")
      return ReadStatus::NOT_IMPLEMENTED;
    } else {
      bool validData = false;
      do {
        CmdExeStatus tempCmdExeStatus;
        outStatus = io_->cmdExeStatusReader.value()->read(&tempCmdExeStatus);
        switch (outStatus) {
          case ReadStatus::SUCCESS:
          case ReadStatus::DISPOSED:
            if (!isValidExeStatus(tempCmdExeStatus, header_)) break;

            validData = true;
            *outCmdExeStatus = tempCmdExeStatus;
            break;
        }
      } while (!validData && outStatus != ReadStatus::NO_DATA);
    }

    return outStatus;
  }

  //! \brief Determines if the current command has terminated by reading any/all corresponding statuses.
  //! If the session is determined to be terminal, then the session will be closed.
  //! \return boolean True if the current command has terminated, false otherwise
  bool hasCommandTerminated() {
    while (isCommandAlive_) {
      switch (CmdStatus tempCmdStatus; read(&tempCmdStatus)) {
        case ReadStatus::SUCCESS:
        case ReadStatus::DISPOSED:
          break;  // Read a valid status that could have been terminal.
        default:
          UMAA_LOG_DEBUG(
              util::SYSTEM_LOGGER,
              "Terminal status for command with session " << header_.sessionId << " has not been received yet")
          return false;  // Terminal state not found.
      }
    }

    return true;
  }

  //! \brief Function to open a command session to start sending commands and reading status samples
  //! \return boolean success
  bool openCommandSession() {
    if (isSessionOpen_) {
      UMAA_LOG_DEBUG(util::SYSTEM_LOGGER, "Session " << header_.sessionId << " already open")
      return false;
    }

    if (!hasCommandTerminated()) {
      UMAA_LOG_DEBUG(util::SYSTEM_LOGGER, "Cannot open new session when there is an active command")
      return false;
    }

    header_.sessionId = NIL_GUID;

    // Run implementation defined session set up
    if (sessionSetUp() == SendStatus::ERROR) {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Failed to run overridden session setup")
      return false;
    }

    header_.sessionId = UuidFactory::getInstance().generateGuid();

    isSessionOpen_ = true;
    return isSessionOpen_;
  }

  //! \brief Function to close a command session and dispose the active command.
  //!        This action is seen as a cancel request if the command is not in a terminal state.
  //! \return SendStatus enum
  SendStatus closeCommandSession() {
    if (!isSessionOpen_) {
      UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Cannot close command session without an open session")
      return SendStatus::ERROR;
    }

    // Run implementation defined session tear down
    if (sessionTearDown() == SendStatus::ERROR) {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Failed to run overridden session tear down")
      return SendStatus::ERROR;
    }

    if (disposeCommand() != SendStatus::SUCCESS) {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Failed to dispose command while closing session")
      return SendStatus::ERROR;
    }

    isSessionOpen_ = false;
    return SendStatus::SUCCESS;
  }

  //! \brief Getter function for the current session ID in use
  //! \return Current sessionID or NIL_GUID if session closed
  NumericGuid getSessionId() const { return header_.sessionId; }

  //! \brief Getter function for checking if the consumer has an open command session
  //! \return bool isCommandSessionOpen
  bool isCommandSessionOpen() const { return isSessionOpen_; }

  //! \brief Get the current command header used by this consumer
  //! \return CommandHeader struct
  CommandHeader getCmdHeader() const { return header_; }

  //! \brief Set the command header data. Will return false if called with an open command session
  //! \param header
  //! \return boolean success/fail
  bool setCmdHeader(const CommandHeader& header) {
    if (isCommandSessionOpen()) {
      UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Cannot update the consumer command header with an active session open")
      return false;
    }

    header_ = header;
    return true;
  }

  //! \brief Set the destination GUID to use for the command consumer. Will return false if called with an open session
  //! \param destination
  //! \return boolean success/fail
  bool setCmdDestination(const UMAA::Common::IdentifierType& destination) {
    if (isCommandSessionOpen()) {
      UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Cannot update the consumer command header with an active session open")
      return false;
    }

    header_.destinationId = destination;
    return true;
  }

 protected:
  CommandHeader header_;
  std::atomic_bool isSessionOpen_ = ATOMIC_VAR_INIT(false);
  std::atomic_bool isCommandAlive_ = ATOMIC_VAR_INIT(false);
  std::shared_ptr<UmaaCommandConsumerIo<CmdType, CmdAck, CmdStatus, CmdExeStatus>> io_;

 private:
  //! \brief Disposes the current command
  //! \return SendStatus enum
  SendStatus disposeCommand() {
    UMAA_LOG_DEBUG(util::SYSTEM_LOGGER, "Disposing command with session " << header_.sessionId)

    CmdType cancelCmd;
    cancelCmd.source().id(header_.sourceId.id());
    cancelCmd.destination().id(header_.destinationId.id());
    cancelCmd.sessionID(header_.sessionId.getGuid());
    cancelCmd.timeStamp(arlcore::umaa::getTimestamp());

    return io_->cmdSender->dispose(cancelCmd);
  }
};

}  // namespace arlcore::umaa::services
#endif  // INCLUDE_UMAA_SERVICES_BASE_COMMANDCONSUMERBASE_H_
