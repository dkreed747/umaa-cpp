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

#ifndef INCLUDE_LOG_SERVICE_LOGREPORTSERVICE_H_
#define INCLUDE_LOG_SERVICE_LOGREPORTSERVICE_H_

#include <string>
#include <memory>
#include <dds/dds.hpp>
#include <UMAA/SO/LogReport/LogReportType.hpp>
#include <UMAA/Common/IdentifierType.hpp>
#include "NumericGuid.h"
#include "CycloneQosProviderWrapper.h"

using LogReportType_t = UMAA::SO::LogReport::LogReportType;
using UMAA::Common::IdentifierType;

namespace arlcore {

//! \brief Log Report Service implementation
class LogReportService {
 public:
  //! \brief Delete default constructor
  LogReportService() = delete;

  //! \brief Constructor
  //! \param particpant
  //! \param qosProvider
  //! \param qosProfile
  //! \param topic
  //! \param source
  LogReportService(const dds::domain::DomainParticipant& participant,
      const std::shared_ptr<arlcore::io::CycloneQosProviderWrapper>& qosProvider,
      const std::string& qosProfile,
      const std::string& topic,
      const arlcore::NumericGuid& id);

  //! \brief Destructor used to dispose the instance if found
  ~LogReportService();

  //! \brief Builds the UMAA log report and sends over the DDS bus
  //! \param entry The string of the log message
  //! \param level The level to log (INFO, ERROR, WARN)
  void buildLogReport(const std::string& entry, const std::string& level);

 private:
  IdentifierType logServiceSourceId_;

  const std::string& topicName_ = UMAA::SO::LogReport::LogReportTypeTopic;

  dds::pub::DataWriter<LogReportType_t> logReportWriter_ = dds::core::null;
  LogReportType_t logReport_;
};

}  // namespace arlcore

#endif  // INCLUDE_LOG_SERVICE_LOGREPORTSERVICE_H_
