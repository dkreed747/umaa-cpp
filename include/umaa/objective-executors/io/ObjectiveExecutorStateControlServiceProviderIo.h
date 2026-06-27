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

#ifndef INCLUDE_UMAA_OBJECTIVE_EXECUTORS_IO_OBJECTIVEEXECUTORSTATECONTROLSERVICEPROVIDERIO_H_
#define INCLUDE_UMAA_OBJECTIVE_EXECUTORS_IO_OBJECTIVEEXECUTORSTATECONTROLSERVICEPROVIDERIO_H_

#include <memory>

#include "UMAA/MM/ObjectiveExecutorControl/ObjectiveExecutorStateCommandType.hpp"
#include "UMAA/MM/ObjectiveExecutorControl/ObjectiveExecutorStateCommandAckReportType.hpp"
#include "UMAA/MM/ObjectiveExecutorControl/ObjectiveExecutorStateCommandStatusType.hpp"
#include "UMAA/MM/ObjectiveExecutorControl/ObjectiveExecutorExecutionStatusReportType.hpp"

#include "UmaaCommandProviderIo.h"

namespace arlcore::umaa {

using UMAA::MM::ObjectiveExecutorControl::ObjectiveExecutorStateCommandType;
using UMAA::MM::ObjectiveExecutorControl::ObjectiveExecutorStateCommandAckReportType;
using UMAA::MM::ObjectiveExecutorControl::ObjectiveExecutorStateCommandStatusType;
using UMAA::MM::ObjectiveExecutorControl::ObjectiveExecutorExecutionStatusReportType;

using UMAA::MM::ObjectiveExecutorControl::ObjectiveExecutorStateCommandTypeTopic;
using UMAA::MM::ObjectiveExecutorControl::ObjectiveExecutorStateCommandAckReportTypeTopic;
using UMAA::MM::ObjectiveExecutorControl::ObjectiveExecutorStateCommandStatusTypeTopic;
using UMAA::MM::ObjectiveExecutorControl::ObjectiveExecutorExecutionStatusReportTypeTopic;

//! \brief ObjectiveExecutorControl service
class ObjectiveExecutorStateControlServiceProviderIo : public arlcore::umaa::domain::UmaaCommandProviderIo<
  ObjectiveExecutorStateCommandType,
  ObjectiveExecutorStateCommandAckReportType,
  ObjectiveExecutorStateCommandStatusType,
  ObjectiveExecutorExecutionStatusReportType> {
 public:
  //! \brief Constructor to pass in all pointers to readers and writers for the objective control service
  //! This function is defined with the SenderBase and ReaderBase objects but will be passed middleware specific
  //! Readers and Senders at runtime
  //! \param ObjExeCmdReader Concrete reader object for objective commands
  //! \param ObjExeCmdAckSender Concrete sender object for objective command acknowledgements
  //! \param ObjExeCmdStatusSender Concrete sender object for command statuses
  //! \param ObjExeStatusSender Concrete sender object for objective execution statuses
  ObjectiveExecutorStateControlServiceProviderIo(
    std::shared_ptr<arlcore::io::ReaderBase<ObjectiveExecutorStateCommandType>> ObjExeCmdReader,
    std::shared_ptr<arlcore::io::SenderBase<ObjectiveExecutorStateCommandAckReportType>> ObjExeCmdAckSender,
    std::shared_ptr<arlcore::io::SenderBase<ObjectiveExecutorStateCommandStatusType>> ObjExeCmdStatusSender,
    std::shared_ptr<arlcore::io::SenderBase<ObjectiveExecutorExecutionStatusReportType>> ObjExeStatusSender) :
    UmaaCommandProviderIo(
      ObjExeCmdReader,
      ObjExeCmdAckSender,
      ObjExeCmdStatusSender,
      ObjExeStatusSender) {}
};

}  // namespace arlcore::umaa
#endif  // INCLUDE_UMAA_OBJECTIVE_EXECUTORS_IO_OBJECTIVEEXECUTORSTATECONTROLSERVICEPROVIDERIO_H_
