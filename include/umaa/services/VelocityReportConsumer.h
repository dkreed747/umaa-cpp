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

#ifndef INCLUDE_UMAA_SERVICES_VELOCITYREPORTCONSUMER_H_
#define INCLUDE_UMAA_SERVICES_VELOCITYREPORTCONSUMER_H_

#include <UMAA/SA/VelocityStatus/VelocityReportType.hpp>

#include "ReportConsumer.h"

namespace arlcore::umaa::services {

using VelocityReportConsumer = ReportConsumer<UMAA::SA::VelocityStatus::VelocityReportType>;

}  // namespace arlcore::umaa::services
#endif  // INCLUDE_UMAA_SERVICES_VELOCITYREPORTCONSUMER_H_
