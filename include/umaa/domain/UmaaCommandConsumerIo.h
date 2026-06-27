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

#ifndef INCLUDE_UMAA_DOMAIN_UMAACOMMANDCONSUMERIO_H_
#define INCLUDE_UMAA_DOMAIN_UMAACOMMANDCONSUMERIO_H_

#include <memory>

#include "ReaderBase.h"
#include "SenderBase.h"

namespace arlcore::umaa::domain {

//! \brief Logical grouping of all IO objects used by a base UMAA command service consumer
//! This includes a sender of command types and three readers. One for receiving acknowledgements from the provider,
//! one for receiving command statuses from the provider, and one for receiving command execution statuses.
//! Note that not all commanded services define an execution status type. A null type or a dummy type can be used in
//! the template in these cases.
//! \tparam CmdType the type of the Command - Must be a UMAACommand or code will not compile
//! \tparam CmdAck the type of the Command Acknowledgement
//! \tparam CmdStatus the type of the Command Status
//! \tparam CmdExeStatus the type of the Command Execution Status
template <class CmdType, class CmdAck, class CmdStatus, class CmdExeStatus = std::nullptr_t>
class UmaaCommandConsumerIo {
 public:
  //! \brief Constructor
  //! \param commandSender
  //! \param commandAckReader
  //! \param commandStatusReader
  //! \param commandExecutionStatusReader
  UmaaCommandConsumerIo(
    std::shared_ptr<arlcore::io::SenderBase<CmdType>> commandSender,
    std::shared_ptr<arlcore::io::ReaderBase<CmdAck>> commandAckReader,
    std::shared_ptr<arlcore::io::ReaderBase<CmdStatus>> commandStatusReader,
    std::optional<std::shared_ptr<arlcore::io::ReaderBase<CmdExeStatus>>> commandExecutionStatusReader = std::nullopt) :
    cmdSender(commandSender),
    cmdAckReader(commandAckReader),
    cmdStatusReader(commandStatusReader),
    cmdExeStatusReader(commandExecutionStatusReader) {}

  const std::shared_ptr<arlcore::io::SenderBase<CmdType>> cmdSender;
  const std::shared_ptr<arlcore::io::ReaderBase<CmdAck>> cmdAckReader;
  const std::shared_ptr<arlcore::io::ReaderBase<CmdStatus>> cmdStatusReader;
  const std::optional<std::shared_ptr<arlcore::io::ReaderBase<CmdExeStatus>>> cmdExeStatusReader;
};

}  // namespace arlcore::umaa::domain
#endif  // INCLUDE_UMAA_DOMAIN_UMAACOMMANDCONSUMERIO_H_
