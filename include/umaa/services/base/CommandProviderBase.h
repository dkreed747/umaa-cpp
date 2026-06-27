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

#ifndef INCLUDE_UMAA_SERVICES_BASE_COMMANDPROVIDERBASE_H_
#define INCLUDE_UMAA_SERVICES_BASE_COMMANDPROVIDERBASE_H_

#include <atomic>
#include <algorithm>
#include <future>
#include <memory>
#include <optional>
#include <queue>
#include <string>
#include <utility>
#include <map>
#include <vector>

#include "CommandProviderSession.h"
#include "CommandStateMachine.h"
#include "Logger.h"
#include "UmaaCommand.h"
#include "UmaaCommandProviderIo.h"
#include "UuidFactory.h"
#include "DestinationReaderFilter.h"
#include "UmaaUtils.h"

using arlcore::NumericGuid;
using arlcore::umaa::domain::UmaaCommandProviderIo;
using arlcore::io::ReaderBase;
using arlcore::io::SenderBase;
using arlcore::io::ReadStatus;
using arlcore::io::SendStatus;

namespace arlcore::umaa::services {

enum class IncomingCommandBehavior {
  QUEUE_INCOMING,
  ACCEPT_CONCURRENT,
  CANCEL_EXISTING,
  COMPLETE_EXISTING,
  REJECT_INCOMING
};

//! \brief The result returned by user-defined logic to signal when to advance
//! OK: No error, but stay in the current state
//! ADVANCE: No error, advance to the next state
//! ERROR: An error occurred, we can no longer continue
enum class CommandStateResult {
  OK,
  ADVANCE,
  ERROR
};

//! \brief A base class that provides the methods used by all UMAA command service providers
//! \tparam CmdType the type of the Command
//! \tparam CmdAck the type of the Command Acknowledgement
//! \tparam CmdStatus the type of the Command Status
//! \tparam CmdExeStatus the type of the Command Execution Status if one exists (Defaults to nullptr)
template <class CmdType, class CmdAck, class CmdStatus, class CmdExeStatus = std::nullptr_t>
class CommandProviderBase {
 public:
  //! \brief Base Constructor
  //! \param source The source ID to receive commands from
  //! \param io Command service provider io containing all readers and writers needed for the service
  //! \param behavior The behavior to use when a new command is received before the previous command finishes
  CommandProviderBase(
    const NumericGuid& source,
    std::shared_ptr<UmaaCommandProviderIo<CmdType, CmdAck, CmdStatus, CmdExeStatus>> io,
    IncomingCommandBehavior behavior = IncomingCommandBehavior::CANCEL_EXISTING) :
    source_(source), behavior_(behavior), io_(io) {}

  using CmdSession = CommandProviderSession<CmdType, CmdAck, CmdStatus, CmdExeStatus>;

  //! \brief Main method of the service provider that executes one cycle of performing command provider tasks
  //! \return bool success/failure
  bool cycle() {
    // Run logic that user defined to run at the start of every cycle
    if (!onCycle() && !activeCommands_.empty()) {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Provider failed to run onCycle() logic")
      std::for_each(activeCommands_.begin(), activeCommands_.end(),
          [&] (std::pair<NumericGuid, std::shared_ptr<CmdSession>> pair) {
        handleFailure(pair.second, CommandStatusReasonEnumType::SERVICE_FAILED,
          "Provider failed to run onCycle() logic");
      });
      activeCommands_.clear();
      return false;
    }

    if (!handleIncomingCommand()) {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Provider failed to handle an incoming command")
      std::for_each(activeCommands_.begin(), activeCommands_.end(),
          [&] (std::pair<NumericGuid, std::shared_ptr<CmdSession>> pair) {
        handleFailure(pair.second, CommandStatusReasonEnumType::SERVICE_FAILED,
          "Provider failed to handle an incoming command");
      });
      activeCommands_.clear();
      return false;
    }

    if (activeCommands_.empty()) {
      UMAA_LOG_TRACE(util::SYSTEM_LOGGER, "Provider state: IDLE")
      return true;
    }

    /* if (hasNewCommandBeenReceived_) {
      UMAA_LOG_INFO(util::SYSTEM_LOGGER, "Provider state: ISSUED")
      hasNewCommandBeenReceived_ = false;
      return true;
    } */

    // Place to store all command sessions that are in their final states while we are iterating
    std::vector<NumericGuid> finalStateSessions;

    bool cycleStatus = std::all_of(activeCommands_.begin(), activeCommands_.end(),
      [&] (std::pair<NumericGuid, std::shared_ptr<CmdSession>> session) {
        CommandStatusEnumType prev;
        bool result = true;
        do {
          prev = session.second->getCommandStatus();
          result = cycleSession(session.second, &finalStateSessions);
        } while (result && prev != session.second->getCommandStatus());
        return result;
        // return cycleSession(session.second, &finalStateSessions);
      });

    std::for_each(finalStateSessions.begin(), finalStateSessions.end(), [&] (NumericGuid session) {
      UMAA_LOG_DEBUG(util::SYSTEM_LOGGER, "Completed session with id " << session)
      activeCommands_.erase(session);
    });

    if (behavior_ == IncomingCommandBehavior::QUEUE_INCOMING && activeCommands_.empty() && !commandQueue_.empty()) {
      std::shared_ptr<CmdSession> cmd = commandQueue_.front();
      commandQueue_.erase(commandQueue_.begin());
      activeCommands_.insert_or_assign(NumericGuid(cmd->getSessionId()), cmd);
      UMAA_LOG_DEBUG(util::SYSTEM_LOGGER, "Added command from queue to active commands: " <<
        cmd->getSessionId())
    }

    return cycleStatus;
  }

  //! \brief Get the behavior used when a new command is received before an existing command finishes
  //! \return enum behavior
  IncomingCommandBehavior getBehavior() const {
    return behavior_;
  }

  //! \brief Set the behavior used when a new command is received before an existing command finishes
  //! \param behavior Behavior to use
  void setBehavior(IncomingCommandBehavior behavior) {
    behavior_ = behavior;
  }

  //! \brief Get the first active command. May be empty optional if the provider is IDLE
  //! \return An active command or nullopt
  std::optional<CmdType> getActiveCommand() const {
    if (activeCommands_.empty()) {
      return std::nullopt;
    }
    return activeCommands_.begin()->second->getCommand();
  }

  //! \brief Get the first active command status. May be empty optional if the provider is IDLE
  //! \return Active command status or nullopt
  std::optional<CommandStatusEnumType> getCommandStatus() const {
    if (activeCommands_.empty()) {
      return std::nullopt;
    }
    return activeCommands_.begin()->second->getCommandStatus();
  }

  //! \brief Get the first active command status reason. May be empty optional if the provider is IDLE
  //! \return Active command status reason or nullopt
  std::optional<CommandStatusReasonEnumType> getCommandStatusReason() const {
    if (activeCommands_.empty()) {
      return std::nullopt;
    }
    return activeCommands_.begin()->second->getCommandStatusReason();
  }

  //! \brief Get a vector containing the currently active command(s).
  //! \return Vector of CmdType, may be empty
  std::vector<CmdType> getActiveCommands() const {
    std::vector<CmdType> commands;
    std::transform(activeCommands_.begin(), activeCommands_.end(), std::back_inserter(commands), [] (auto s) {
        return s.second->getCommand();
    });
    return move(commands);
  }

  //! \brief Get an active command's state (i.e. command status and reason) by its session ID.
  //! May be empty if command does not exist.
  //! \param sessionId The session ID of the command to get the state of
  //! \return An optional pair of the command status and command status reason
  std::optional<std::pair<CommandStatusEnumType, CommandStatusReasonEnumType>> getCommandState(
      const NumericGuid& sessionId) {
    if (activeCommands_.count(sessionId) == 0) {
      return std::nullopt;
    }
    std::shared_ptr<CmdSession> session = activeCommands_.at(sessionId);
    return std::make_pair(session->getCommandStatus(), session->getCommandStatusReason());
  }

  std::vector<CmdType> getQueuedCommands() const {
    std::vector<CmdType> commands;
    std::transform(commandQueue_.begin(), commandQueue_.end(), std::back_inserter(commands), [] (auto s) {
      return s->getCommand();
    });
    return commands;
  }

  bool isQueued(const NumericGuid& sessionId) {
    return std::any_of(commandQueue_.begin(), commandQueue_.end(),
      [&sessionId] (const std::shared_ptr<CmdSession> s) {
        return s->getSessionId() == sessionId;
      });
  }

 protected:
  //! \brief Optionally overridden function to perform any logic that should occur upon transition
  //! to the ISSUED state
  //! \param session The CommandProviderSession of the command to perform logic for
  //! \return CommandStateResult success/failure
  virtual CommandStateResult onIssued(const std::weak_ptr<CmdSession> session) {
    UMAA_LOG_TRACE(util::SYSTEM_LOGGER, "Virtual base method called - not overridden")
    return CommandStateResult::ADVANCE;
  }

  //! \brief Optionally overridden function to perform any logic that should occur upon transition
  //! to the COMMANDED state
  //! \param session The CommandProviderSession of the command to perform logic for
  //! \return CommandStateResult success/failure
  virtual CommandStateResult onCommanded(const std::weak_ptr<CmdSession> session) {
    UMAA_LOG_TRACE(util::SYSTEM_LOGGER, "Virtual base method called - not overridden")
    return CommandStateResult::ADVANCE;
  }

  //! \brief Optionally overridden function to perform any logic that should occur upon transition
  //! to the EXECUTING state
  //! \param session The CommandProviderSession of the command to perform logic for
  //! \return CommandStateResult success/failure
  virtual CommandStateResult onExecuting(const std::weak_ptr<CmdSession> session) {
    UMAA_LOG_TRACE(util::SYSTEM_LOGGER, "Virtual base method called - not overridden")
    return CommandStateResult::OK;
  }

  //! \brief Optionally overridden function to perform any logic that should occur when a new command is received
  //! while actively processing a non-terminal command
  //! \param session The CommandProviderSession of the command to perform logic for
  //! \return bool success/failure
  virtual bool onInterrupted(const std::weak_ptr<CmdSession> session) {
    UMAA_LOG_TRACE(util::SYSTEM_LOGGER, "Virtual base method called - not overridden")
    return true;
  }

  //! \brief Optionally overridden function to perform any logic that should occur upon transition
  //! to the COMPLETED state
  //! \param session The CommandProviderSession of the command to perform logic for
  //! \return bool success/failure
  virtual bool onCompleted(const std::weak_ptr<CmdSession> session) {
    UMAA_LOG_TRACE(util::SYSTEM_LOGGER, "Virtual base method called - not overridden")
    return true;
  }

  //! \brief Optionally overridden function to perform any logic that should occur upon transition
  //! to the CANCELED state
  //! \param session The CommandProviderSession of the command to perform logic for
  //! \return bool success/failure
  virtual bool onCanceled(const std::weak_ptr<CmdSession> session) {
    UMAA_LOG_TRACE(util::SYSTEM_LOGGER, "Virtual base method called - not overridden")
    return true;
  }

  //! \brief Optionally overridden function to perform any logic that should occur upon transition
  //! to the FAILED state
  //! \param session The CommandProviderSession of the command to perform logic for
  //! \return bool success/failure
  virtual bool onFailed(const std::weak_ptr<CmdSession> session) {
    UMAA_LOG_TRACE(util::SYSTEM_LOGGER, "Virtual base method called - not overridden")
    return true;
  }

  //! \brief Optionally overridden function to perform any logic that should occur when an active command is updated
  //! \param session The CommandProviderSession of the command to perform logic for
  //! \param previousCmd The command from before the update
  //! \param updatedCmd The command after the update
  //! \return bool success/failure
  virtual bool onUpdated(const std::weak_ptr<CmdSession> session, const CmdType& previousCmd,
      const CmdType& updatedCmd) {
    UMAA_LOG_TRACE(util::SYSTEM_LOGGER, "Virtual base method called - not overridden")
    return true;
  }

  //! \brief Optionally overridden function to perform any validation logic against an incoming command
  //! validation occurs before transitioning to the COMMANDED state. Returning false indicates a transition
  //! to the VALIDATION_FAILED state
  //! \param cmd The command to perform validation on
  //! \return bool command valid/invalid
  virtual bool isCommandValid(const CmdType& cmd) {
    UMAA_LOG_TRACE(util::SYSTEM_LOGGER, "Virtual base method called - not overridden")
    return true;
  }

  //! \brief Optionally overridden function to check if a command has completed successfully
  //! \param session The CommandProviderSession of the command to check for completion of
  //! \return bool command complete/not complete
  virtual bool isCommandCompleted(const std::weak_ptr<CmdSession> session) {
    UMAA_LOG_TRACE(util::SYSTEM_LOGGER, "Virtual base method called - not overridden")
    return false;
  }

  //! \brief Optionally overridden function to check if a command has failed
  //! \param session The CommandProviderSession of the command to check for failure
  //! \return UMAA CommandStatusReasonEnumType return succeeded if not failed
  virtual CommandStatusReasonEnumType isCommandFailed(const std::weak_ptr<CmdSession> session) {
    UMAA_LOG_TRACE(util::SYSTEM_LOGGER, "Virtual base method called - not overridden")
    return CommandStatusReasonEnumType::SUCCEEDED;
  }

  //! \brief Optionally overridden function to perform any logic that should occur every loop of the provider
  //! \return bool success/failure
  virtual bool onCycle() {
    UMAA_LOG_TRACE(util::SYSTEM_LOGGER, "Virtual base method called - not overridden")
    return true;
  }

  //! \brief Optionally overridden function to read incoming commands
  //! Base implementation provided
  //! \param outCommand pointer to the command data to overwrite
  //! \return ReadStatus enum
  virtual ReadStatus read(CmdType *outCommand) {
    return io_->cmdReader->read(outCommand);
  }

  //! \brief Optionally overridden function to send command execution status
  //! \param cmd The command to send execution status for
  //! \return SendStatus enum
  virtual SendStatus sendExecutionStatus(const CmdType& cmd) {
    UMAA_LOG_TRACE(util::SYSTEM_LOGGER, "Virtual base method called - not overridden")
    return SendStatus::NOT_IMPLEMENTED;
  }

  //! \brief Optionally overridden function to dispose samples related to command execution status
  //! \param cmd The command to dispose execution status for
  //! \return SendStatus enum
  virtual SendStatus disposeExecutionStatus(const CmdType& cmd) {
    UMAA_LOG_TRACE(util::SYSTEM_LOGGER, "Virtual base method called - not overridden")
    return SendStatus::NOT_IMPLEMENTED;
  }

  NumericGuid source_;
  std::shared_ptr<UmaaCommandProviderIo<CmdType, CmdAck, CmdStatus, CmdExeStatus>> io_;

 private:
  //! \brief Cycle a single Command Session
  //! \param session Pointer to the session to cycle
  //! \param finalStateSessions Pointer to a vector of Session IDs to add the provided session's ID to if said
  //! session is in a final state
  //! \return Whether the session could be cycled successfully
  bool cycleSession(std::shared_ptr<CmdSession> session, std::vector<NumericGuid>* finalStateSessions) {
    UMAA_LOG_DEBUG(util::SYSTEM_LOGGER, "Command with session ID " << session->getSessionId() <<
      " is in state " << session->getCommandStatus())

    switch (session->getCommandStatus()) {
      case CommandStatusEnumType::ISSUED:
        return handleIssued(session);
      case CommandStatusEnumType::COMMANDED:
        return handleCommanded(session);
      case CommandStatusEnumType::EXECUTING:
        return handleExecuting(session);
      case CommandStatusEnumType::COMPLETED:  // Intentional fallthrough
      case CommandStatusEnumType::CANCELED:
      case CommandStatusEnumType::FAILED:
        finalStateSessions->push_back(session->getSessionId());
        return true;
      default:
        UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Provider attempted to handle unknown UMAA command state")
        return false;
    }
  }

  //! \brief Reads the next command type off the bus and processes it
  //! \paragraph
  //! Case 1. Cancellation Request
  //! Case 2. Update a non-terminal command
  //! Case 3. Completely new command
  //! \return bool success/failure
  bool handleIncomingCommand() {
    auto filter = std::make_shared<arlcore::umaa::DestinationReaderFilter<CmdType>>(source_);
    io_->cmdReader->setManualFilter(filter);

    CmdType incomingCommand;
    ReadStatus status = read(&incomingCommand);

    if (status == ReadStatus::ERROR || status == ReadStatus::NOT_IMPLEMENTED) {
      return false;
    }

    // No new command on bus - NOOP
    if (status == ReadStatus::NO_DATA || status == ReadStatus::INVALID_DATA) {
      return true;
    }

    NumericGuid incomingSessionId(incomingCommand.sessionID());

    // CASE 1. Check for and handle a cancellation request
    if (status == ReadStatus::DISPOSED) {
      if (activeCommands_.count(incomingSessionId) > 0) {
        UMAA_LOG_DEBUG(util::SYSTEM_LOGGER, "Canceling session " << incomingSessionId)
        std::shared_ptr<CmdSession> session = activeCommands_.at(incomingSessionId);
        if (!commandQueue_.empty()) {
          activeCommands_.insert_or_assign(commandQueue_.front()->getSessionId(), commandQueue_.front());
          commandQueue_.erase(commandQueue_.begin());
        }
        return handleCancel(session) && activeCommands_.erase(incomingSessionId);
      } else if (isQueued(incomingSessionId)) {
        auto it = std::find_if(commandQueue_.begin(), commandQueue_.end(), [&incomingSessionId] (auto s) {
          return s->getSessionId() == incomingSessionId;
        });

        if (it == commandQueue_.end()) {
          UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Unable to get queued command session with ID: " << incomingSessionId)
          return false;
        }

        auto session = *it;
        commandQueue_.erase(it);

        return handleCancel(session);
      } else {
        UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Got a cancellation request for a command with unknown session ID " <<
          incomingSessionId)
        return true;
      }
    }

    // CASE 2. Check for and handle a new command or an update to an existing command
    if (status == ReadStatus::SUCCESS) {
      if (!activeCommands_.empty()) {
        // If an active command session has the same session ID, update it
        if (activeCommands_.count(incomingSessionId) > 0) {
          std::shared_ptr<CmdSession> session = activeCommands_.at(incomingSessionId);
          return handleUpdate(session, incomingCommand);
        }
        bool allCompleted = false;
        bool allCanceled = false;
        std::shared_ptr<CmdSession> tempSession;
        switch (behavior_) {
          case IncomingCommandBehavior::ACCEPT_CONCURRENT:
            UMAA_LOG_DEBUG(util::SYSTEM_LOGGER,
              "New command received while not idle, handling based on set behavior: `ACCEPT_CONCURRENT`")
            break;
          case IncomingCommandBehavior::CANCEL_EXISTING:
            UMAA_LOG_DEBUG(util::SYSTEM_LOGGER,
              "New command received while not idle, handling based on set behavior: `CANCEL_EXISTING`")
            // Cancel all active commands in case behavior is changed from ACCEPT_CONCURRENT with active commands
            allCanceled = std::all_of(activeCommands_.begin(), activeCommands_.end(), [&](auto pair) {
              return handleCancel(pair.second, "Canceled due to new incoming command");
            });
            // Clearing the map should remove the last reference to the shared object,
            // calling their destructors and disposing their instances
            activeCommands_.clear();

            if (!allCanceled) {
              UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Unable to cancel all active commands")
              return false;
            }
            break;
          case IncomingCommandBehavior::COMPLETE_EXISTING:
            UMAA_LOG_DEBUG(util::SYSTEM_LOGGER,
              "New command received while not idle, handling based on set behavior: `COMPLETE_EXISTING`")
            // Complete all active commands in case behavior is changed from ACCEPT_CONCURRENT with active commands
            allCompleted = std::all_of(activeCommands_.begin(), activeCommands_.end(), [&](auto pair) {
              return pair.second->sendStatus(CommandStatusEnumType::COMPLETED, CommandStatusReasonEnumType::SUCCEEDED,
                "Completed due to new incoming command") == SendStatus::SUCCESS;
            });
            // Clearing the map should remove the last reference to the shared object,
            // calling their destructors and disposing their instances
            activeCommands_.clear();

            if (!allCompleted) {
              UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Unable to complete all active commands")
              return false;
            }
            break;
          case IncomingCommandBehavior::REJECT_INCOMING:
            UMAA_LOG_DEBUG(util::SYSTEM_LOGGER,
              "New command received while not idle, handling based on set behavior: `REJECT_EXISTING`")

            // Make a temporary command session to send status for failing the incoming command
            tempSession = std::make_shared<CmdSession>(source_, incomingCommand, io_,
              std::bind(&CommandProviderBase<CmdType, CmdAck, CmdStatus, CmdExeStatus>::sendExecutionStatus,
                this, std::placeholders::_1),
              std::bind(&CommandProviderBase<CmdType, CmdAck, CmdStatus, CmdExeStatus>::disposeExecutionStatus,
                this, std::placeholders::_1));

            tempSession->sendStatus();
            tempSession->sendAck();
            tempSession->fail(CommandStatusReasonEnumType::SERVICE_FAILED);
            tempSession->sendStatus("Active command already exists, cancel first");
            tempSession.reset();
            // Return early to not add active command
            return true;
          case IncomingCommandBehavior::QUEUE_INCOMING:
              UMAA_LOG_DEBUG(util::SYSTEM_LOGGER, "New command received. Adding to QUEUE position "
                << std::to_string(commandQueue_.size()))

              tempSession = std::make_shared<CmdSession>(source_, incomingCommand, io_,
              std::bind(&CommandProviderBase<CmdType, CmdAck, CmdStatus, CmdExeStatus>::sendExecutionStatus,
                this, std::placeholders::_1),
              std::bind(&CommandProviderBase<CmdType, CmdAck, CmdStatus, CmdExeStatus>::disposeExecutionStatus,
                this, std::placeholders::_1));

              tempSession->sendStatus("Command received, currently waiting behind " +
                std::to_string(commandQueue_.size()) + " other commands.");
              tempSession->sendAck();

              commandQueue_.push_back(tempSession);
              // return early to not add active command
              return true;
              break;
          default:
            UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Command provider has reached impossible state")
            return false;
        }
      }

      // Add the incoming command to activeCommands_
      auto [it, added] = activeCommands_.emplace(incomingSessionId,
        std::make_shared<CmdSession>(source_, incomingCommand, io_,
          std::bind(&CommandProviderBase<CmdType, CmdAck, CmdStatus, CmdExeStatus>::sendExecutionStatus,
            this, std::placeholders::_1),
          std::bind(&CommandProviderBase<CmdType, CmdAck, CmdStatus, CmdExeStatus>::disposeExecutionStatus,
            this, std::placeholders::_1)));

      std::shared_ptr<CmdSession> newSession = it->second;

      if (!added) {
        UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Unable to add incoming command to set of active commands")
        return false;
      }

      if (newSession->sendStatus() == SendStatus::ERROR) {
        UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Error sending status for new command session " <<
          newSession->getSessionId())
        return false;
      }

      if (newSession->sendAck() == SendStatus::ERROR) {
        UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Error sending ack for new command session " <<
          newSession->getSessionId())
        return false;
      }
    }

    return true;
  }

  //! \brief Handle the transition from a non-terminal state to the ISSUED
  //! \param session The CommandProviderSession to update the command of
  //! \param updatedCommand The command to update the CommandProviderSession with
  //! \return bool success/failure
  bool handleUpdate(std::weak_ptr<CmdSession> session, const CmdType& updatedCmd) {
    if (auto cmdSession = session.lock()) {
      CmdType previousCmd = cmdSession->getCommand();
      if (!cmdSession->update(updatedCmd)) {
        UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Failed to update existing command session")
        return false;
      }

      if (!onUpdated(session, previousCmd, updatedCmd)) {
        UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Failed to run user defined onUpdated logic")
        return false;
      }

      if (cmdSession->sendStatus("Command was updated successfully!") == SendStatus::ERROR) {
        UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Error sending status for command session " << cmdSession->getSessionId())
        return false;
      }

      if (!handleIssued(session)) {
        UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Failed while running handleIssued")
        return false;
      }

      // hasNewCommandBeenReceived_ = true;

      return true;
    } else {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Called handleUpdate with invalid session pointer")
      return false;
    }
  }

  //! \brief Handle the transition from the ISSUED state to COMMANDED
  //! \param session The CommandProviderSession of the command to handle the transition of
  //! \param logMessage An optional log message to include with the status update upon state transition
  //! \return bool success/failure
  bool handleIssued(std::weak_ptr<CmdSession> session, const std::string& logMessage = "") {
    if (auto cmdSession = session.lock()) {
      if (cmdSession->getCommandStatus() != CommandStatusEnumType::ISSUED) {
        UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Command session " <<
          cmdSession->getSessionId() << " is in a non-ISSUED state")
        return false;
      }

      // Validate by calling user validation function
      if (!isCommandValid(cmdSession->getCommand())) {
        return handleFailure(session, CommandStatusReasonEnumType::VALIDATION_FAILED);
      }

      if (CommandStateResult result = onIssued(session); result == CommandStateResult::ERROR) {
        UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Failed to run user defined onIssued logic")
        return handleFailure(session, CommandStatusReasonEnumType::SERVICE_FAILED, "Failed to run onIssued logic");
      } else if (result == CommandStateResult::ADVANCE) {
        if (!cmdSession->advanceState()) {
          UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Internal failure moving from ISSUED to COMMANDED")
          return false;
        }

        if (cmdSession->sendStatus(logMessage) == SendStatus::ERROR) {
          UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Error sending status for command session " << cmdSession->getSessionId())
          return false;
        }
      }

      return true;
    } else {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Called handleIssued with invalid session pointer")
      return false;
    }
  }

  //! \brief Handle the transition from the COMMANDED state to the EXECUTING state
  //! \param session The CommandProviderSession of the command to handle the transition of
  //! \param logMessage An optional log message to include with the status update upon state transition
  //! \return bool success/failure
  bool handleCommanded(std::weak_ptr<CmdSession> session, const std::string& logMessage = "") {
    if (auto cmdSession = session.lock()) {
      if (cmdSession->getCommandStatus() != CommandStatusEnumType::COMMANDED) {
         UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Command session " <<
          cmdSession->getSessionId() << " is in a non-COMMANDED state")
      // if (cmdSession->getCommandStatus() != CommandStatusEnumType::COMMANDED || !cmdSession->advanceState()) {
      //   UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Internal failure moving from COMMANDED to EXECUTING")
        return false;
      }

      // if (!onExecuting(session)) {
      //   UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Failed to run user defined onExecuting logic")
      if (CommandStateResult result = onCommanded(session); result == CommandStateResult::ERROR) {
        UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Failed to run user defined onCommanded logic")
        return handleFailure(session, CommandStatusReasonEnumType::SERVICE_FAILED, "Failed to run onCommanded logic");
      } else if (result == CommandStateResult::ADVANCE) {
        if (!cmdSession->advanceState()) {
          UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Internal failure moving from COMMANDED to EXECUTING")
          return false;
        }

        if (cmdSession->sendStatus(logMessage) == SendStatus::ERROR) {
          UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Error sending status for command session " << cmdSession->getSessionId())
          return false;
        }
      }

      return true;
    } else {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Called handleCommanded with invalid session pointer")
      return false;
    }
  }

  bool handleExecuting(std::weak_ptr<CmdSession> session, const std::string& logMessage = "") {
    if (auto cmdSession = session.lock()) {
      if (cmdSession->getCommandStatus() != CommandStatusEnumType::EXECUTING) {
         UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Command session " <<
          cmdSession->getSessionId() << " is in a non-EXECUTING state")
        return false;
      }

      if (CommandStateResult result = onExecuting(session); result == CommandStateResult::ERROR) {
        UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Failed to run user defined onExecuting logic")
        return handleFailure(session, CommandStatusReasonEnumType::SERVICE_FAILED, "Failed to run onExecuting logic");
      } else if (CommandStatusReasonEnumType reason = isCommandFailed(session);
                reason != CommandStatusReasonEnumType::SUCCEEDED) {
        UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Command with session ID " << cmdSession->getSessionId() <<
          " failed with reason " << reason)
        return handleFailure(session, reason);
      } else if (result == CommandStateResult::ADVANCE || isCommandCompleted(session)) {
        return handleCompleted(session);
      }

      if (cmdSession->sendExecutionStatus() == SendStatus::ERROR) {
        UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Error sending execution status for command session " <<
          cmdSession->getSessionId())
        return false;
      }

      return true;
    } else {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Called handleExecuting with invalid session pointer")
      return false;
    }
  }

  //! \brief Handle the transition from the EXECUTING state to the COMPLETED state
  //! \param session The CommandProviderSession of the command to handle the transition of
  //! \param logMessage An optional log message to include with the status update upon state transition
  //! \return bool success/failure
  bool handleCompleted(std::weak_ptr<CmdSession> session, const std::string& logMessage = "") {
    if (auto cmdSession = session.lock()) {
      if (cmdSession->getCommandStatus() != CommandStatusEnumType::EXECUTING || !cmdSession->advanceState()) {
        UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Internal failure moving from EXECUTING to COMPLETED")
        return false;
      }

      if (!onCompleted(session)) {
        UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Failed to run user defined onCompleted logic")
        return false;
      }

      if (cmdSession->sendStatus(logMessage) == SendStatus::ERROR) {
        UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Error sending status for command session " << cmdSession->getSessionId())
        return false;
      }

      if (cmdSession->sendExecutionStatus() == SendStatus::ERROR) {
        UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Error sending execution status for command session " <<
          cmdSession->getSessionId())
        return false;
      }

      return true;
    } else {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Called handleCompleted with invalid session pointer")
      return false;
    }
  }

  //! \brief Handle the transition from a non-terminal state to the CANCELED state
  //! \param session The CommandProviderSession of the command to handle the transition of
  //! \param logMessage An optional log message to include with the status update upon state transition
  //! \return  bool success/failure
  bool handleCancel(std::weak_ptr<CmdSession> session, const std::string& logMessage = "") {
    if (auto cmdSession = session.lock()) {
      if (!cmdSession->cancel()) {
        UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Cancel command received for terminal or MIA command session "
          << cmdSession->getSessionId())
        return true;
      }

      if (!onCanceled(session)) {
        UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Failed to run user defined onCanceled logic")
        return false;
      }

      if (cmdSession->sendStatus(logMessage) == SendStatus::ERROR) {
        UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Error sending status for command session " << cmdSession->getSessionId())
        return false;
      }

      if (cmdSession->sendExecutionStatus() == SendStatus::ERROR) {
        UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Error sending execution status for command session " <<
          cmdSession->getSessionId())
        return false;
      }
      return true;
    } else {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Called handleCancel with invalid session pointer")
      return false;
    }
  }

  //! \brief Handle the transition from a non-terminal state to the FAILED state
  //! \param session The CommandProviderSession of the command to handle the transition of
  //! \param failureReason the CommandStatusReasonEnumType associated with the failure
  //! \param logMessage An optional log message to include with the status update upon state transition
  //! \return Whether the failure case was handled successfully
  bool handleFailure(std::weak_ptr<CmdSession> session, const CommandStatusReasonEnumType& failureReason,
      const std::string& logMessage = "") {
    UMAA_LOG_DEBUG(util::SYSTEM_LOGGER, "Failed called with reason: " << failureReason)
    if (auto cmdSession = session.lock()) {
      if (!cmdSession->fail(failureReason)) {
        UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Unable to transition session " << cmdSession->getSessionId() <<
          " to failed state")
        return false;
      }

      if (!onFailed(session)) {
        UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Failed to run user defined onFailed logic")
        return false;
      }

      if (cmdSession->sendStatus(logMessage) == SendStatus::ERROR) {
        UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Error sending status for command session " << cmdSession->getSessionId())
        return false;
      }

      if (cmdSession->sendExecutionStatus() == SendStatus::ERROR) {
        UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Error sending execution status for command session " <<
          cmdSession->getSessionId())
        return false;
      }

      return true;
    } else {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Called handleFailure with invalid session pointer")
      return false;
    }
  }

  IncomingCommandBehavior behavior_;
  // bool hasNewCommandBeenReceived_ = false;
  std::map<NumericGuid, std::shared_ptr<CmdSession>> activeCommands_;
  std::vector<std::shared_ptr<CmdSession>> commandQueue_;
};

}  // namespace arlcore::umaa::services
#endif  // INCLUDE_UMAA_SERVICES_BASE_COMMANDPROVIDERBASE_H_
