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

#ifndef INCLUDE_HEALTH_STATUS_HEALTHSTATUSCONTROL_H_
#define INCLUDE_HEALTH_STATUS_HEALTHSTATUSCONTROL_H_

#include <string>
#include <memory>
#include <thread>
#include <utility>
#include <unordered_map>
#include <dds/dds.hpp>

#include <UMAA/SO/HealthReport/HealthReportType.hpp>

#include "HealthStatusData.h"
#include "SenderBase.h"
#include "CycloneSender.h"
#include "UuidFactory.h"
#include "UmaaUtils.h"

using UMAA::SO::HealthReport::HealthReportType;
using UMAA::SO::HealthReport::HealthReportTypeTopic;
using UMAA::Common::MaritimeEnumeration::ErrorCodeEnumModule::ErrorCodeEnumType;
using UMAA::Common::MaritimeEnumeration::ErrorConditionEnumModule::ErrorConditionEnumType;

namespace std {
template<>
struct hash<UMAA::Common::IdentifierType> {
  size_t operator()(const UMAA::Common::IdentifierType& identifierType) const {
    std::size_t h1 = arlcore::umaa::hashNumericGuid(identifierType.id());
    std::size_t h2 = arlcore::umaa::hashNumericGuid(identifierType.parentID());
    return h1 ^ (h2 << 1);
  }
};

template<>
struct hash<std::pair<UMAA::Common::IdentifierType, ErrorCodeEnumType>> {
  size_t operator()(const pair<UMAA::Common::IdentifierType, ErrorCodeEnumType>& key) const {
    std::size_t h1 = std::hash<UMAA::Common::IdentifierType>{}(key.first);
    std::size_t h2 = arlcore::HealthStatusData::errorCodeToInt(key.second);
    return h1 ^ (h2 << 1);
  }
};
};  // namespace std

namespace arlcore {

class HealthStatusControl {
 public:
  //! \brief Health Report Service Implementation
  HealthStatusControl() = delete;

  //! \brief Constructor that creates a Health Report Writer from parameters
  //! \param participant
  //! \param publisher
  //! \param writerQos
  //! \param heartbeatMS The desired rate to report health in milliseconds
  HealthStatusControl(const dds::domain::DomainParticipant& participant,
    const dds::pub::Publisher& publisher,
    const dds::pub::qos::DataWriterQos& writerQos,
    const UMAA::Common::IdentifierType& sourceId,
    const uint32_t heartbeatMS = 1000);

  //! \brief Constructor that receives a pre-made SenderBase.
  //! \param sourceId           The unique identifier of the HealthReport Service.
  //! \param healthReportWriter A SenderBase that sends HealthReportType.
  //! \param heartbeatMS        The desired rate to report health in milliseconds.
  HealthStatusControl(const UMAA::Common::IdentifierType& sourceId,
    std::shared_ptr<arlcore::io::SenderBase<HealthReportType>> healthReportWriter,
    const uint32_t heartbeatMS = 1000);

  //! Destructor used to dispose the instance if found.
  //! Upon destruction, the Health Monitoring Thread is stopped and Health Reports are disposed.
  ~HealthStatusControl();

  //! A convenience function to initialize a ResourceID/ErrorCode pair with a ErrorConditionEnumType::NONE.
  //! If the ResourceID/ErrorCode pair is already being reported, this function will do nothing.
  //! \param resourceId ID of the resource to report.
  //! \param code       The type of system associated with the error report.
  void registerResource(UMAA::Common::IdentifierType resourceId, ErrorCodeEnumType code);

  //! Convenience function to stop a ResourceID/Code pair from being reported.
  //! Disposes the HealthReport associated with the ResourceID/Code.
  //! \param resourceId ID of the resource to report.
  //! \param code       The type of system associated with the error report.
  void deregisterResource(UMAA::Common::IdentifierType resourceId, ErrorCodeEnumType code);

  //! \brief Updates and publishes the health status of a ResourceId/Code pair
  //! \param resourceId The ID of the resource to report.
  //! \param code       The type of system associated with the error report.
  //! \param severity   The type of error reported.
  //! \param status     A detailed string which specifies the status of the system such as the reason for failure.
  void updateSystemHealth(const UMAA::Common::IdentifierType& resourceId,
              const ErrorCodeEnumType& code,
              const ErrorConditionEnumType& severity,
              const std::string& status);

  //! Starts the Health Report Publishing Thread.
  //! This thread will send all stored health reports at a user-specified rate.
  void startThread();

  //! Stops the Health Report Publishing Thread.
  void stopThread();

 private:
  //! \brief Builds a Health Report with the current time.
  //! \param resourceId The type of system associated with the error report.
  //! \param severity   The ID of the resource to report.
  //! \param code       The type of error reported.
  //! \param status     A detailed string which specifies the status of the system such as the reason for failure.
  std::shared_ptr<HealthReportType> buildReportWithCurrentTime(
                                      const UMAA::Common::IdentifierType& resourceId,
                                      const ErrorCodeEnumType& code,
                                      const ErrorConditionEnumType& severity,
                                      const std::string& status) const;

  //! \brief Sends a single HealthReport.
  void sendReport(const std::shared_ptr<HealthReportType>& report) const;

  //! \brief Sends all health reports stored in the service provider
  void sendAllReports() const;

  //! \brief Function that calls sendAllReports() at a specified rate.
  void healthReportThread() const;

  /*! The unique identifier for the Health Report service provider.*/
  UMAA::Common::IdentifierType sourceId_;
  /*! The HealthReportType Data Writer*/
  std::shared_ptr<arlcore::io::SenderBase<HealthReportType>> healthReportWriter_;
  /*! The rate at which to send Health Reports when the thread starts. */
  uint32_t heartbeatMS_;
  /*! The Health Service execution thread */
  std::thread healthServiceThread_;
  /*! True if the thread is running, otherwise false. */
  bool isThreadRunning_ = false;

  /*! A map to store a <ResourceID, ErrorCodeEnumType> : HealthReportType pair */
  std::unordered_map<std::pair<UMAA::Common::IdentifierType, ErrorCodeEnumType>,
              std::shared_ptr<HealthReportType>> mostRecentHealthReports_;
};

}  // namespace arlcore
#endif  // INCLUDE_HEALTH_STATUS_HEALTHSTATUSCONTROL_H_
