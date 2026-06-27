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

#ifndef INCLUDE_UMAA_DOMAIN_ASYNCCOMMANDSESSION_H_
#define INCLUDE_UMAA_DOMAIN_ASYNCCOMMANDSESSION_H_

#include <atomic>
#include <condition_variable>
#include <future>
#include <mutex>
#include <optional>
#include <utility>

#include "UMAA/Common/MaritimeEnumeration/MaritimeEnumerationSets.hpp"

#include "NumericGuid.h"

using UMAA::Common::MaritimeEnumeration::CommandStatusEnumModule::CommandStatusEnumType;
using UMAA::Common::MaritimeEnumeration::CommandStatusReasonEnumModule::CommandStatusReasonEnumType;

namespace arlcore::umaa::domain {

//! \brief Wrapper class around std::future that provides an interface for monitoring (consumer) and updating (provider)
//! Command Sessions
class AsyncCommandSession {
 public:
  //! \brief Main constructor needs a std::future source either returned from std::promise or std::async
  //! \param future the future to wrap as a command session
  explicit AsyncCommandSession(std::future<CommandStatusEnumType>&& future);

  // No copy construction, move construction is okay
  AsyncCommandSession(const AsyncCommandSession&) = default;
  AsyncCommandSession& operator=(const AsyncCommandSession&) = default;
  AsyncCommandSession(AsyncCommandSession&&) = default;
  AsyncCommandSession& operator=(AsyncCommandSession&&) = default;

  //! \brief Check if the command has been acknowledged
  //! \return true if acknowledged
  bool isCommandAcknowledged() const;

  //! \brief Set the acknowledgement status of the session to true
  void setAsAcknowledged();

  //! \brief Request that the asynchronous command session be canceled
  void requestCancel();

  //! \brief Check if the consumer has submitted a cancel request
  //! \return true if the provider should cancel the command session
  bool hasCancelBeenRequested() const;

  //! \brief Get the current command status associated with the command session
  //! \return Current command status or nullopt if nothing has been received
  std::optional<CommandStatusEnumType> getStatus() const;

  //! \brief Set the current command status associated with the command session
  //! \param status
  void setStatus(const std::optional<CommandStatusEnumType>& status);

  //! \brief Get the current status reason associated with the command session
  //! \return status
  std::optional<CommandStatusReasonEnumType> getStatusReason() const;

  //! \brief Set the current command status reason associated with the command session
  //! \param statusReason
  void setStatusReason(const std::optional<CommandStatusReasonEnumType> &statusReason);

  //! \brief Get the current ID associated with the command session
  //! \return Current session ID or NIL if not set
  arlcore::NumericGuid getSessionId() const;

  //! \brief Set the current ID associated with the command session
  //! \param sessionId
  void setSessionId(const arlcore::NumericGuid& sessionId);

  //! \brief Exposed interface for std::future::get()
  //! \return true if the future is still valid
  bool valid() const;

  //! \brief Get the value associated with the fulfilled future
  //! Note this function will block until the result is valid
  //! \return Terminal command status
  CommandStatusEnumType get();

  //! \brief Wait for the result to be set on the future indefinitely
  void wait() const;

  //! \brief Wait for the result to be valid for a number of milliseconds
  //! \param duration milliseconds to wait
  //! \return status of the wrapped future
  std::future_status wait_for(const std::chrono::milliseconds& duration) const;

  //! \brief Wait until a time point is reached for the result to be valid
  //! \param timeoutTime Chrono time_point to wait until
  //! \return status of the wrapped future
  std::future_status wait_until(const std::chrono::steady_clock::time_point& timeoutTime) const;

  //! \brief Wait for the session to receive an acknowledgement indefinitely
  void waitForAck() const;

  //! \brief Wait for the session to receive an acknowledgement for a number of milliseconds
  //! \param duration milliseconds to wait
  //! \return true if the session has been acknowledged
  bool waitForAck_for(const std::chrono::milliseconds& duration) const;

  //! \brief Wait until a time point is reached for the session to be acknowledged
  //! \param timeoutTime Chrono time_point to wait until
  //! \return true if the session has been acknowledged
  bool waitForAck_until(const std::chrono::steady_clock::time_point& timeoutTime) const;

  //! \brief Wait for the session to receive a status message that equals a desired state indefinitely
  //! \param status State to wait for
  void waitForStatus(const CommandStatusEnumType& status) const;

  //! \brief Wait for the session to receive a status message that equals a desired state for a number of milliseconds
  //! \param status State to wait for
  //! \param duration milliseconds to wait
  //! \return true if the status has been reached in the wait period
  bool waitForStatus_for(const CommandStatusEnumType& status, const std::chrono::milliseconds& duration) const;

  //! \brief Wait until a time point is reached for the session to receive a status message that equals a desired state
  //! \param status State to wait for
  //! \param timeoutTime Chrono time_point to wait until
  //! \return true if the status has been reached in the wait period
  bool waitForStatus_until(
    const CommandStatusEnumType& status, const std::chrono::steady_clock::time_point& timeoutTime) const;

  //! \brief Wait for the consumer to cancel the session
  void waitForCancel() const;

  //! \brief Wait for the consumer of the session to cancel for a number of milliseconds
  //! \param duration milliseconds to wait
  //! \return true if the session has been canceled within the wait period
  bool waitForCancel_for(const std::chrono::milliseconds& duration) const;

  //! \brief Wait until the consumer of the session cancels the session
  //! \param timeoutTime Chrono time_point to wait until
  //! \return true if the session has been canceled within the wait period
  bool waitForCancel_until(const std::chrono::steady_clock::time_point& timeoutTime) const;

 private:
  std::future<CommandStatusEnumType> future_;
  mutable std::mutex sessionMutex_;
  mutable std::condition_variable sessionCondVar_;

  bool cancelRequested_ = false;
  bool ack_ = false;


  std::optional<CommandStatusEnumType> status_ = std::nullopt;
  std::optional<CommandStatusReasonEnumType> statusReason_ = std::nullopt;
  arlcore::NumericGuid sessionId_ = arlcore::NIL_GUID;
};

}  // namespace arlcore::umaa::domain
#endif  // INCLUDE_UMAA_DOMAIN_ASYNCCOMMANDSESSION_H_
