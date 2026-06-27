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

#include "AsyncCommandSession.h"

namespace arlcore::umaa::domain {

AsyncCommandSession::AsyncCommandSession(std::future<CommandStatusEnumType>&& future) :
  future_(std::move(future)) {}

bool AsyncCommandSession::isCommandAcknowledged() const {
  std::scoped_lock<std::mutex> lock(sessionMutex_);
  return ack_;
}

void AsyncCommandSession::setAsAcknowledged() {
  {
    std::scoped_lock<std::mutex> lock(sessionMutex_);
    ack_ = true;
  }
  sessionCondVar_.notify_all();
}

void AsyncCommandSession::requestCancel() {
  {
    std::scoped_lock<std::mutex> lock(sessionMutex_);
    cancelRequested_ = true;
  }
  sessionCondVar_.notify_all();
}

bool AsyncCommandSession::hasCancelBeenRequested() const {
  std::scoped_lock<std::mutex> lock(sessionMutex_);
  return cancelRequested_;
}

std::optional<CommandStatusEnumType> AsyncCommandSession::getStatus() const {
  std::scoped_lock<std::mutex> lock(sessionMutex_);
  return status_;
}

void AsyncCommandSession::setStatus(const std::optional<CommandStatusEnumType>& status) {
  {
    std::scoped_lock<std::mutex> lock(sessionMutex_);
    status_ = status;
  }
  sessionCondVar_.notify_all();
}

std::optional<CommandStatusReasonEnumType> AsyncCommandSession::getStatusReason() const {
  std::scoped_lock<std::mutex> lock(sessionMutex_);
  return statusReason_;
}

void AsyncCommandSession::setStatusReason(const std::optional<CommandStatusReasonEnumType> &statusReason) {
  {
    std::scoped_lock<std::mutex> lock(sessionMutex_);
    statusReason_ = statusReason;
  }
  sessionCondVar_.notify_all();
}

arlcore::NumericGuid AsyncCommandSession::getSessionId() const {
  std::scoped_lock<std::mutex> lock(sessionMutex_);
  return sessionId_;
}

void AsyncCommandSession::setSessionId(const arlcore::NumericGuid& sessionId) {
  {
    std::scoped_lock<std::mutex> lock(sessionMutex_);
    sessionId_ = sessionId;
  }
  sessionCondVar_.notify_all();
}

bool AsyncCommandSession::valid() const {
  std::scoped_lock<std::mutex> lock(sessionMutex_);
  return future_.valid();
}

CommandStatusEnumType AsyncCommandSession::get() {
  std::scoped_lock<std::mutex> lock(sessionMutex_);
  return future_.get();
}

void AsyncCommandSession::wait() const {
  future_.wait();
}

std::future_status AsyncCommandSession::wait_for(const std::chrono::milliseconds& duration) const {
  return future_.wait_for(duration);
}

std::future_status AsyncCommandSession::wait_until(const std::chrono::steady_clock::time_point& timeoutTime) const {
  return future_.wait_until(timeoutTime);
}

void AsyncCommandSession::waitForAck() const {
  std::unique_lock lock(sessionMutex_);
  sessionCondVar_.wait(lock, [this] { return ack_; });
}

bool AsyncCommandSession::waitForAck_for(const std::chrono::milliseconds& duration) const {
  std::unique_lock lock(sessionMutex_);
  return sessionCondVar_.wait_for(lock, duration, [this] { return ack_; });
}

bool AsyncCommandSession::waitForAck_until(const std::chrono::steady_clock::time_point& timeoutTime) const {
  std::unique_lock lock(sessionMutex_);
  return sessionCondVar_.wait_until(lock, timeoutTime, [this] { return ack_; });
}

void AsyncCommandSession::waitForStatus(const CommandStatusEnumType& status) const {
  std::unique_lock lock(sessionMutex_);
  sessionCondVar_.wait(lock, [this, status] { return status_ == status; });
}

bool AsyncCommandSession::waitForStatus_for(const CommandStatusEnumType& status,
  const std::chrono::milliseconds& duration) const {
    std::unique_lock lock(sessionMutex_);
    return sessionCondVar_.wait_for(lock, duration, [this, status] { return status_ == status; });
}

bool AsyncCommandSession::waitForStatus_until(
  const CommandStatusEnumType& status, const std::chrono::steady_clock::time_point& timeoutTime) const {
    std::unique_lock lock(sessionMutex_);
    return sessionCondVar_.wait_until(lock, timeoutTime, [this, status] { return status_ == status; });
}

void AsyncCommandSession::waitForCancel() const {
  std::unique_lock lock(sessionMutex_);
  sessionCondVar_.wait(lock, [this] { return cancelRequested_; });
}

bool AsyncCommandSession::waitForCancel_for(const std::chrono::milliseconds& duration) const {
    std::unique_lock lock(sessionMutex_);
    return sessionCondVar_.wait_for(lock, duration, [this] { return cancelRequested_; });
}

bool AsyncCommandSession::waitForCancel_until(const std::chrono::steady_clock::time_point& timeoutTime) const {
  std::unique_lock lock(sessionMutex_);
  return sessionCondVar_.wait_until(lock, timeoutTime, [this] { return cancelRequested_; });
}

}  // namespace arlcore::umaa::domain
