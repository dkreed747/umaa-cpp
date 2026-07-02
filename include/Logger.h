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

#ifndef INCLUDE_LOGGER_H_
#define INCLUDE_LOGGER_H_

#include <log4cxx/consoleappender.h>
#include <log4cxx/log4cxx.h>
#include <log4cxx/logger.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/rolling/rollingfileappender.h>
#include <log4cxx/xml/domconfigurator.h>
#include <log4cxx/spi/configurator.h>
#include <string>
#include <atomic>
#include <cstdlib>
#include <sstream>
#include <memory>
#include <optional>
#include <dds/dds.hpp>

#include "CycloneQosProviderWrapper.h"
#include "LogReportService.h"
#include "CycloneUtilities.h"
#include "UuidFactory.h"

namespace util {

#ifndef UMAA_LOG_TRACE
#define UMAA_LOG_TRACE(logger, message) { \
  LOG4CXX_TRACE(logger, message); }
#endif  // UMAA_LOG_TRACE

#ifndef UMAA_LOG_DEBUG
#define UMAA_LOG_DEBUG(logger, message) { \
  LOG4CXX_DEBUG(logger, message); }
#endif  // UMAA_LOG_DEBUG

#ifndef UMAA_LOG_INFO
#define UMAA_LOG_INFO(logger, message) { \
          LOG4CXX_INFO(logger, message); \
          ::log4cxx::helpers::MessageBuffer oss_; \
          oss_.str(oss_ << message); \
          util::sendDds("INFO", oss_.str(oss_)); }
#endif  // UMAA_LOG_INFO

#ifndef UMAA_LOG_WARN
#define UMAA_LOG_WARN(logger, message) { \
          LOG4CXX_WARN(logger, message); \
          ::log4cxx::helpers::MessageBuffer oss_; \
          oss_.str(oss_ << message); \
          util::sendDds("WARN", oss_.str(oss_)); }
#endif  // UMAA_LOG_WARN

#ifndef UMAA_LOG_ERROR
#define UMAA_LOG_ERROR(logger, message) { \
          LOG4CXX_ERROR(logger, message); \
          ::log4cxx::helpers::MessageBuffer oss_; \
          oss_.str(oss_ << message); \
          util::sendDds("ERROR", oss_.str(oss_)); }
#endif  // UMAA_LOG_ERROR

template <typename T>
static bool tryGetEnvVariable(const std::string& variableName, T& variable) {  // NOLINT - This needs to be a non-const reference
  char* value = std::getenv(variableName.c_str());
  if (value == nullptr) {
    return false;
  }

  std::istringstream iss(value);
  if (iss >> variable) {
    return true;
  }
  return false;
}

template <typename T>
static std::optional<T> tryGetEnvVariable(const std::string& variableName) {
  char* value = std::getenv(variableName.c_str());
  if (value == nullptr) {
    return std::nullopt;
  }

  std::istringstream iss(value);
  T variable;
  iss >> variable;
  return std::optional<T>(variable);
}

// Intentionally leaked at process exit: destroying the service from a static destructor
// would serialize DDS samples after glibc has already run thread_local destructors (the
// cyclonedds-cxx get_type_props<T>() cache), which is a use-after-free on shutdown.
static std::shared_ptr<arlcore::LogReportService>* logReportSvcHolder = nullptr;
static std::atomic<bool> logReportInstantiated = ATOMIC_VAR_INIT(false);

static void defaultFactory(const log4cxx::LoggerPtr& rootLogger) {
  log4cxx::PatternLayoutPtr patternPtr =
      std::make_shared<log4cxx::PatternLayout>("[%p] %d{HH:mm:ss.SSSS} %M - %m%n");
  log4cxx::ConsoleAppenderPtr consoleAppenderPtr = std::make_shared<log4cxx::ConsoleAppender>(patternPtr);

  rootLogger->addAppender(consoleAppenderPtr);
  rootLogger->setLevel(log4cxx::Level::getInfo());
}

static log4cxx::LoggerPtr getLogger() {
  static const log4cxx::LoggerPtr log = log4cxx::Logger::getRootLogger();
  static std::atomic<bool> firstTimeCalled(true);
  if (firstTimeCalled) {
    log4cxx::xml::DOMConfigurator::configure("log4cxx.xml");
    if (log->getAllAppenders().size() == 0) {
      defaultFactory(log);
      LOG4CXX_WARN(log, "Logger configured from default factory");
    } else {
      LOG4CXX_INFO(log, "Logger configured from log4cxx.xml");
    }
    firstTimeCalled = false;
  }
  return log;
}

static const log4cxx::LoggerPtr SYSTEM_LOGGER = getLogger();

static void setupLogService(const std::string& level, const std::string& message) {
  int32_t domainId = tryGetEnvVariable<int32_t>("domain-id").value_or(0);
  LOG4CXX_TRACE(SYSTEM_LOGGER, "Setting up Log Report with domainID: " << std::to_string(domainId));

  std::string qosURI = tryGetEnvVariable<std::string>("domain-qos-file")
                        .value_or("CYCLONE_QOS_PROFILES.xml");
  LOG4CXX_TRACE(SYSTEM_LOGGER, "Setting up Log Report with qosURI: " << qosURI);

  std::string qosProfile = tryGetEnvVariable<std::string>("domain-qos-profile")
                              .value_or("UMAA_QoS_Library::UMAA_Base_Profile");
  LOG4CXX_TRACE(SYSTEM_LOGGER, "Setting up Log Report with qosProfile: " << qosProfile);

  arlcore::NumericGuid sourceId;
  if (auto id_str = tryGetEnvVariable<std::string>("SOURCE_ID")) {
    sourceId = arlcore::UuidFactory::getInstance().parseGuidFromString(id_str.value());
  } else {
    sourceId = arlcore::UuidFactory::getInstance().generateGuid();
  }
  LOG4CXX_TRACE(SYSTEM_LOGGER, "Setting up Log Report with sourceID: " << sourceId);


  dds::domain::DomainParticipant participant = arlcore::io::getDomainParticipant(domainId);

  const auto qosProvider = std::make_shared<arlcore::io::CycloneQosProviderWrapper>(qosURI, qosProfile);

  const std::string& topicName = UMAA::SO::LogReport::LogReportTypeTopic;

  logReportSvcHolder = new std::shared_ptr<arlcore::LogReportService>(
      std::make_shared<arlcore::LogReportService>(
          participant, qosProvider, qosProfile, topicName, sourceId));

  usleep(10000);  // 0.01 seconds for DDS discovery
  (*logReportSvcHolder)->buildLogReport(message, level);
  LOG4CXX_INFO(SYSTEM_LOGGER, "Log Report Service successfully instantiated.");
  logReportInstantiated = true;
}

static void sendDds(const std::string& level, const std::string& message) {
  if (logReportInstantiated) {
    (*logReportSvcHolder)->buildLogReport(message, level);
  } else {
    setupLogService(level, message);
  }
}

}  // namespace util
#endif  // INCLUDE_LOGGER_H_
