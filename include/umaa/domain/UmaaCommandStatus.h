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

#ifndef INCLUDE_UMAA_DOMAIN_UMAACOMMANDSTATUS_H_
#define INCLUDE_UMAA_DOMAIN_UMAACOMMANDSTATUS_H_


#include <string>

#include "EnumTypes.h"
#include "UmaaCommandStatusBase.h"

namespace arl {

class UmaaCommandStatus : public UmaaCommandStatusBase {
 public:
  UmaaCommandStatus() = default;
  UmaaCommandStatus(
      const NumericGUID_t &source,
      const NumericGUID_t &session_id,
      const CommandState &command_status,
      const CommandStatus &command_status_reason,
      const std::string &log_message) :
    UmaaCommandStatusBase(source, session_id),
    command_status_(command_status),
    command_status_reason_(command_status_reason),
    log_message_(log_message) {}

  inline CommandState getCommandStatus() const { return command_status_; }
  inline CommandStatus getCommandStatusReason() const { return command_status_reason_; }
  inline std::string getLogMessage() const { return log_message_; }

  inline void setCommandStatus(const CommandState &command_status) { command_status_ = command_status; }
  inline void setCommandStatusReason(const CommandStatus &command_status_reason) {
      command_status_reason_ = command_status_reason;
  }
  inline void setLogMessage(const std::string &log_message) { log_message_ = log_message; }

 private:
  CommandState command_status_ = CommandState::INVALID;
  CommandStatus command_status_reason_ = CommandStatus::SERVICE_FAILED;
  std::string log_message_ = "";
};

}  // namespace arl

#endif  // INCLUDE_UMAA_DOMAIN_UMAACOMMANDSTATUS_H_
