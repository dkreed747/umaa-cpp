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

#ifndef INCLUDE_UMAA_OBJECTIVE_EXECUTORS_SERVICES_GLOBALHOVERCONTROLSERVICECONSUMER_H_
#define INCLUDE_UMAA_OBJECTIVE_EXECUTORS_SERVICES_GLOBALHOVERCONTROLSERVICECONSUMER_H_

#include <memory>

#include "UMAA/MO/GlobalHoverControl/GlobalHoverCommandType.hpp"
#include "UMAA/MO/GlobalHoverControl/GlobalHoverCommandAckReportType.hpp"
#include "UMAA/MO/GlobalHoverControl/GlobalHoverCommandStatusType.hpp"
#include "UMAA/MO/GlobalHoverControl/GlobalHoverExecutionStatusReportType.hpp"

#include "CommandConsumerBase.h"
#include "GlobalHoverControlServiceConsumerIo.h"

namespace arlcore::umaa {

//! \brief Concrete implementation or a Global Hover command consumer
using GlobalHoverControlServiceConsumer = arlcore::umaa::services::CommandConsumerBase<
  GlobalHoverCommandType,
  GlobalHoverCommandAckReportType,
  GlobalHoverCommandStatusType,
  GlobalHoverExecutionStatusReportType>;

}  // namespace arlcore::umaa
#endif  // INCLUDE_UMAA_OBJECTIVE_EXECUTORS_SERVICES_GLOBALHOVERCONTROLSERVICECONSUMER_H_
