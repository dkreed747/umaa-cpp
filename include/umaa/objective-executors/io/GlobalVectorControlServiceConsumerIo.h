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

#ifndef INCLUDE_UMAA_OBJECTIVE_EXECUTORS_IO_GLOBALVECTORCONTROLSERVICECONSUMERIO_H_
#define INCLUDE_UMAA_OBJECTIVE_EXECUTORS_IO_GLOBALVECTORCONTROLSERVICECONSUMERIO_H_

#include "UMAA/MO/GlobalVectorControl/GlobalVectorCommandType.hpp"
#include "UMAA/MO/GlobalVectorControl/GlobalVectorCommandAckReportType.hpp"
#include "UMAA/MO/GlobalVectorControl/GlobalVectorCommandStatusType.hpp"
#include "UMAA/MO/GlobalVectorControl/GlobalVectorExecutionStatusReportType.hpp"

#include "UmaaCommandConsumerIo.h"

namespace arlcore::umaa {

using UMAA::MO::GlobalVectorControl::GlobalVectorCommandType;
using UMAA::MO::GlobalVectorControl::GlobalVectorCommandAckReportType;
using UMAA::MO::GlobalVectorControl::GlobalVectorCommandStatusType;
using UMAA::MO::GlobalVectorControl::GlobalVectorExecutionStatusReportType;

using UMAA::MO::GlobalVectorControl::GlobalVectorCommandTypeTopic;
using UMAA::MO::GlobalVectorControl::GlobalVectorCommandAckReportTypeTopic;
using UMAA::MO::GlobalVectorControl::GlobalVectorCommandStatusTypeTopic;
using UMAA::MO::GlobalVectorControl::GlobalVectorExecutionStatusReportTypeTopic;

// Command consumer base IO defined with Global Vector types
using GlobalVectorControlServiceConsumerIo = arlcore::umaa::domain::UmaaCommandConsumerIo<
  GlobalVectorCommandType,
  GlobalVectorCommandAckReportType,
  GlobalVectorCommandStatusType,
  GlobalVectorExecutionStatusReportType>;

}  // namespace arlcore::umaa
#endif  // INCLUDE_UMAA_OBJECTIVE_EXECUTORS_IO_GLOBALVECTORCONTROLSERVICECONSUMERIO_H_
