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


#include <string>
#include <dds/dds.hpp>
#include <UMAA/SO/LogReport/LogReportType.hpp>

#include "CycloneUtilities.h"
#include "LogReportService.h"
#include "UuidFactory.h"
#include "UmaaUtils.h"

namespace arlcore {

  LogReportService::LogReportService(const dds::domain::DomainParticipant& participant,
      const std::shared_ptr<arlcore::io::CycloneQosProviderWrapper>& qosProvider,
      const std::string& qosProfile,
      const std::string& topic,
      const arlcore::NumericGuid& id) {
        logServiceSourceId_ = IdentifierType(
          id,
          arlcore::NIL_GUID);
        auto ddsTopic = arlcore::io::getTopic<LogReportType_t>(participant, topic);
        dds::pub::Publisher default_pub(participant, qosProvider->publisher_qos(qosProfile));
        logReportWriter_ = dds::pub::DataWriter<LogReportType_t>(default_pub,
            ddsTopic, qosProvider->datawriter_qos(qosProfile));
      }


    LogReportService::~LogReportService() {
      auto instanceHandle = logReportWriter_.lookup_instance(logReport_);

      // Only dispose if the instance is found. Otherwise, do nothing
      if (instanceHandle != dds::core::InstanceHandle::nil()) {
        logReportWriter_.dispose_instance(instanceHandle);
      }
    }

  void LogReportService::buildLogReport(const std::string& entry, const std::string& level) {
    if (level == "ERROR") {
      logReport_.level(UMAA::Common::MaritimeEnumeration::LogLevelEnumModule::LogLevelEnumType::ERROR);
    } else if (level == "INFO") {
      logReport_.level(UMAA::Common::MaritimeEnumeration::LogLevelEnumModule::LogLevelEnumType::INFORMATION);
    } else if (level == "WARN") {
      logReport_.level(UMAA::Common::MaritimeEnumeration::LogLevelEnumModule::LogLevelEnumType::WARNING);
    } else {
      return;
    }

    logReport_.entry(entry);
    logReport_.timeStamp(arlcore::umaa::getTimestamp());
    logReport_.source(logServiceSourceId_);

    logReportWriter_.write(logReport_);
  }

}  // namespace arlcore
