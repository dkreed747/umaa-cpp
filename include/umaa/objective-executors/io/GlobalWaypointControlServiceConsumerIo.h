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

#ifndef INCLUDE_UMAA_OBJECTIVE_EXECUTORS_IO_GLOBALWAYPOINTCONTROLSERVICECONSUMERIO_H_
#define INCLUDE_UMAA_OBJECTIVE_EXECUTORS_IO_GLOBALWAYPOINTCONTROLSERVICECONSUMERIO_H_

#include <memory>

#include "UMAA/MO/GlobalWaypointControl/GlobalWaypointCommandType.hpp"
#include "UMAA/MO/GlobalWaypointControl/GlobalWaypointCommandAckReportType.hpp"
#include "UMAA/MO/GlobalWaypointControl/GlobalWaypointCommandStatusType.hpp"
#include "UMAA/MO/GlobalWaypointControl/GlobalWaypointExecutionStatusReportType.hpp"

#include "UmaaCommandConsumerIo.h"

namespace arlcore::umaa {

using UMAA::MO::GlobalWaypointControl::GlobalWaypointType;
using UMAA::MO::GlobalWaypointControl::GlobalWaypointCommandType;
using UMAA::MO::GlobalWaypointControl::GlobalWaypointCommandAckReportType;
using UMAA::MO::GlobalWaypointControl::GlobalWaypointCommandStatusType;
using UMAA::MO::GlobalWaypointControl::GlobalWaypointExecutionStatusReportType;
using UMAA::MO::GlobalWaypointControl::GlobalWaypointCommandTypeWaypointsListElement;

using UMAA::MO::GlobalWaypointControl::GlobalWaypointCommandTypeTopic;
using UMAA::MO::GlobalWaypointControl::GlobalWaypointCommandAckReportTypeTopic;
using UMAA::MO::GlobalWaypointControl::GlobalWaypointCommandStatusTypeTopic;
using UMAA::MO::GlobalWaypointControl::GlobalWaypointExecutionStatusReportTypeTopic;
using UMAA::MO::GlobalWaypointControl::GlobalWaypointCommandTypeWaypointsListElementTopic;

//! \brief Derive waypoint specific IO with extra large list sender
class GlobalWaypointControlServiceConsumerIo : public arlcore::umaa::domain::UmaaCommandConsumerIo<
  GlobalWaypointCommandType,
  GlobalWaypointCommandAckReportType,
  GlobalWaypointCommandStatusType,
  GlobalWaypointExecutionStatusReportType> {
 public:
  //! \brief Constructor to pass in all pointers to readers and writers for the waypoint control service
  //! This function is defined with the SenderBase and ReaderBase objects but will be passed middleware specific
  //! Readers and Senders at runtime
  //! \param waypointCommandSender Concrete sender object for waypoint commands
  //! \param waypointCommandAckReader Concrete reader object for waypoint acknowledgements
  //! \param waypointCommandStatusReader Concrete reader object for waypoint command statuses
  //! \param waypointCommandExecutionStatusReader Concrete reader for waypoint execution statuses
  //! \param waypointCommandListElementSender Concrete sender for sending large list waypoint elements
  GlobalWaypointControlServiceConsumerIo(
    std::shared_ptr<arlcore::io::SenderBase<GlobalWaypointCommandType>> waypointCommandSender,
    std::shared_ptr<arlcore::io::ReaderBase<GlobalWaypointCommandAckReportType>> waypointCommandAckReader,
    std::shared_ptr<arlcore::io::ReaderBase<GlobalWaypointCommandStatusType>> waypointCommandStatusReader,
    std::shared_ptr<arlcore::io::ReaderBase<GlobalWaypointExecutionStatusReportType>>
      waypointCommandExecutionStatusReader,
    std::shared_ptr<arlcore::io::SenderBase<GlobalWaypointCommandTypeWaypointsListElement>>
      waypointCommandListElementSender) :
    UmaaCommandConsumerIo(
      waypointCommandSender,
      waypointCommandAckReader,
      waypointCommandStatusReader,
      waypointCommandExecutionStatusReader),
      listElementSender(waypointCommandListElementSender) {}

  const std::shared_ptr<arlcore::io::SenderBase<GlobalWaypointCommandTypeWaypointsListElement>> listElementSender;
};

}  // namespace arlcore::umaa
#endif  // INCLUDE_UMAA_OBJECTIVE_EXECUTORS_IO_GLOBALWAYPOINTCONTROLSERVICECONSUMERIO_H_
