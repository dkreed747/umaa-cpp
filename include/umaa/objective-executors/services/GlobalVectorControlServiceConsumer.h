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

#ifndef INCLUDE_UMAA_OBJECTIVE_EXECUTORS_SERVICES_GLOBALVECTORCONTROLSERVICECONSUMER_H_
#define INCLUDE_UMAA_OBJECTIVE_EXECUTORS_SERVICES_GLOBALVECTORCONTROLSERVICECONSUMER_H_

#include <memory>

#include "UMAA/MO/GlobalVectorControl/GlobalVectorCommandType.hpp"
#include "UMAA/MO/GlobalVectorControl/GlobalVectorCommandAckReportType.hpp"
#include "UMAA/MO/GlobalVectorControl/GlobalVectorCommandStatusType.hpp"
#include "UMAA/MO/GlobalVectorControl/GlobalVectorExecutionStatusReportType.hpp"

#include "CommandConsumerBase.h"
#include "GlobalVectorControlServiceConsumerIo.h"

namespace arlcore::umaa {

//! \brief Concrete implementation or a Global Vector command consumer
using GlobalVectorControlServiceConsumer = arlcore::umaa::services::CommandConsumerBase<
  GlobalVectorCommandType,
  GlobalVectorCommandAckReportType,
  GlobalVectorCommandStatusType,
  GlobalVectorExecutionStatusReportType>;

}  // namespace arlcore::umaa
#endif  // INCLUDE_UMAA_OBJECTIVE_EXECUTORS_SERVICES_GLOBALVECTORCONTROLSERVICECONSUMER_H_
