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

#include "HealthStatusControl.h"

namespace arlcore {

  HealthStatusControl::HealthStatusControl(const dds::domain::DomainParticipant& participant,
      const dds::pub::Publisher& publisher,
      const dds::pub::qos::DataWriterQos& writerQos,
      const UMAA::Common::IdentifierType& sourceId,
      const uint32_t heartbeatMS) : sourceId_(sourceId), heartbeatMS_(heartbeatMS) {
    healthReportWriter_ = std::make_shared<arlcore::io::CycloneSender<HealthReportType>>(participant,
                                                                                          publisher,
                                                                                          HealthReportTypeTopic,
                                                                                          writerQos);
  }

  HealthStatusControl::HealthStatusControl(const UMAA::Common::IdentifierType& sourceId,
    std::shared_ptr<arlcore::io::SenderBase<HealthReportType>> healthReportWriter,
    const uint32_t heartbeatMS) :
      sourceId_(sourceId), healthReportWriter_(healthReportWriter), heartbeatMS_(heartbeatMS) {
  }

  HealthStatusControl::~HealthStatusControl() {
    for (const auto& [key, report] : mostRecentHealthReports_) {
      healthReportWriter_->dispose(*report);
    }
    mostRecentHealthReports_.clear();
    stopThread();
  }

  /*
   * Public Methods 
  */

  void HealthStatusControl::registerResource(UMAA::Common::IdentifierType resourceId, ErrorCodeEnumType code) {
    std::string status = "No error condition exists.";
    auto key = std::make_pair(resourceId, code);
    // If the report was registered successfully, send a report.
    if (auto healthReport = buildReportWithCurrentTime(resourceId, code, ErrorConditionEnumType::NONE, status);
            mostRecentHealthReports_.try_emplace(key, healthReport).second) {
      sendReport(healthReport);
      return;
    }
    UMAA_LOG_WARN(util::SYSTEM_LOGGER, "ResourceID has already been registered")
  }

  void HealthStatusControl::deregisterResource(UMAA::Common::IdentifierType resourceId,
                                                ErrorCodeEnumType code) {
    auto key = std::make_pair(resourceId, code);
    try {
      auto report = mostRecentHealthReports_.at(key);
      healthReportWriter_->dispose(*report);
      mostRecentHealthReports_.erase(key);
    } catch (const std::out_of_range &e) {
      UMAA_LOG_WARN(util::SYSTEM_LOGGER, "ResourceID/ErrorCode pair is not being reported. No resource deregistered.")
    }
  }

  void HealthStatusControl::updateSystemHealth(const UMAA::Common::IdentifierType& resourceId,
                const ErrorCodeEnumType& code,
                const ErrorConditionEnumType& severity,
                const std::string& status) {
    std::shared_ptr<HealthReportType> healthReport = buildReportWithCurrentTime(resourceId, code, severity, status);
    std::pair<UMAA::Common::IdentifierType, ErrorCodeEnumType> key = std::make_pair(resourceId, code);
    mostRecentHealthReports_[key] = healthReport;

    sendReport(healthReport);
  }

void HealthStatusControl::startThread() {
  if (!isThreadRunning_) {
    UMAA_LOG_DEBUG(util::SYSTEM_LOGGER, "Starting monitor thread")
    isThreadRunning_ = true;
    healthServiceThread_ = std::thread(&HealthStatusControl::healthReportThread, this);
  }
}

void HealthStatusControl::stopThread() {
  if (isThreadRunning_) {
    isThreadRunning_ = false;
    if (healthServiceThread_.joinable()) {
      healthServiceThread_.join();
      UMAA_LOG_DEBUG(util::SYSTEM_LOGGER, "Stopping Monitor Thread")
    }
  }
}

/*
 * Private Methods 
*/

  void HealthStatusControl::sendReport(const std::shared_ptr<HealthReportType>& report) const {
    report.get()->timeStamp(arlcore::umaa::getTimestamp());
    healthReportWriter_->send(*report);
  }

  void HealthStatusControl::sendAllReports() const {
    for (const auto& [key, report] : mostRecentHealthReports_) {
      if (report.get()->severity() == ErrorConditionEnumType::NONE) {
        report.get()->logTime(arlcore::umaa::getTimestamp());
      }
      sendReport(report);
    }
  }

  std::shared_ptr<HealthReportType> HealthStatusControl::buildReportWithCurrentTime(
        const UMAA::Common::IdentifierType& resourceId,
        const ErrorCodeEnumType& code,
        const ErrorConditionEnumType& severity,
        const std::string& status) const {
      auto healthReport = std::make_shared<HealthReportType>();
      UMAA::Common::Measurement::DateTime currentTime = arlcore::umaa::getTimestamp();
      healthReport->timeStamp(currentTime);
      healthReport->source(sourceId_);
      healthReport->logTime(currentTime);
      healthReport->severity(severity);
      healthReport->status(status);
      healthReport->code(code);
      healthReport->resourceID(resourceId);
      return healthReport;
    }

    void HealthStatusControl::healthReportThread() const {
    while (isThreadRunning_) {
      auto absTime = std::chrono::high_resolution_clock::now();
      sendAllReports();
      absTime += std::chrono::milliseconds(heartbeatMS_);
      std::this_thread::sleep_until(absTime);
    }
  }

}  // namespace arlcore
