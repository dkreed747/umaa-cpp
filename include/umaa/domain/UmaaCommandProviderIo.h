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

#ifndef INCLUDE_UMAA_DOMAIN_UMAACOMMANDPROVIDERIO_H_
#define INCLUDE_UMAA_DOMAIN_UMAACOMMANDPROVIDERIO_H_

#include <memory>

#include "ReaderBase.h"
#include "SenderBase.h"

namespace arlcore::umaa::domain {

//! \brief Logical grouping of all IO objects used by a base UMAA command service provider
//! This includes a reader of command types and three writers. One for sending acknowledgements to the consumer,
//! one for sending command statuses to the consumer, and one for sending command execution statuses.
//! Note that not all commanded services define an execution status type. A null type or a dummy type can be used in
//! the template in these cases.
//! \tparam CmdType the type of the command
//! \tparam CmdAck the type of the command acknowledgement
//! \tparam CmdStatus the type of the command status
//! \tparam CmdExeStatus the type of the command execution status
template <class CmdType, class CmdAck, class CmdStatus, class CmdExeStatus = std::nullptr_t>
class UmaaCommandProviderIo {
 public:
  //! \brief Constructor
  //! \param commandReader
  //! \param commandAckSender
  //! \param commandStatusSender
  //! \param commandExecutionStatusSender
  UmaaCommandProviderIo(
    std::shared_ptr<arlcore::io::ReaderBase<CmdType>> commandReader,
    std::shared_ptr<arlcore::io::SenderBase<CmdAck>> commandAckSender,
    std::shared_ptr<arlcore::io::SenderBase<CmdStatus>> commandStatusSender,
    std::optional<std::shared_ptr<arlcore::io::SenderBase<CmdExeStatus>>> commandExecutionStatusSender = std::nullopt) :
    cmdReader(commandReader),
    cmdAckSender(commandAckSender),
    cmdStatusSender(commandStatusSender),
    cmdExeStatusSender(commandExecutionStatusSender) {}

  const std::shared_ptr<arlcore::io::ReaderBase<CmdType>> cmdReader;
  const std::shared_ptr<arlcore::io::SenderBase<CmdAck>> cmdAckSender;
  const std::shared_ptr<arlcore::io::SenderBase<CmdStatus>> cmdStatusSender;
  const std::optional<std::shared_ptr<arlcore::io::SenderBase<CmdExeStatus>>> cmdExeStatusSender;
};

}  // namespace arlcore::umaa::domain
#endif  // INCLUDE_UMAA_DOMAIN_UMAACOMMANDPROVIDERIO_H_
