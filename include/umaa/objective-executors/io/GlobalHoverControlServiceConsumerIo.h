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

#ifndef INCLUDE_UMAA_OBJECTIVE_EXECUTORS_IO_GLOBALHOVERCONTROLSERVICECONSUMERIO_H_
#define INCLUDE_UMAA_OBJECTIVE_EXECUTORS_IO_GLOBALHOVERCONTROLSERVICECONSUMERIO_H_

#include "UMAA/MO/GlobalHoverControl/GlobalHoverCommandType.hpp"
#include "UMAA/MO/GlobalHoverControl/GlobalHoverCommandAckReportType.hpp"
#include "UMAA/MO/GlobalHoverControl/GlobalHoverCommandStatusType.hpp"
#include "UMAA/MO/GlobalHoverControl/GlobalHoverExecutionStatusReportType.hpp"

#include "UmaaCommandConsumerIo.h"

namespace arlcore::umaa {

using UMAA::MO::GlobalHoverControl::GlobalHoverCommandType;
using UMAA::MO::GlobalHoverControl::GlobalHoverCommandAckReportType;
using UMAA::MO::GlobalHoverControl::GlobalHoverCommandStatusType;
using UMAA::MO::GlobalHoverControl::GlobalHoverExecutionStatusReportType;

using UMAA::MO::GlobalHoverControl::GlobalHoverCommandTypeTopic;
using UMAA::MO::GlobalHoverControl::GlobalHoverCommandAckReportTypeTopic;
using UMAA::MO::GlobalHoverControl::GlobalHoverCommandStatusTypeTopic;
using UMAA::MO::GlobalHoverControl::GlobalHoverExecutionStatusReportTypeTopic;

// Command consumer base IO defined with Global Hover types
using GlobalHoverControlServiceConsumerIo = arlcore::umaa::domain::UmaaCommandConsumerIo<
  GlobalHoverCommandType,
  GlobalHoverCommandAckReportType,
  GlobalHoverCommandStatusType,
  GlobalHoverExecutionStatusReportType>;

}  // namespace arlcore::umaa
#endif  // INCLUDE_UMAA_OBJECTIVE_EXECUTORS_IO_GLOBALHOVERCONTROLSERVICECONSUMERIO_H_
