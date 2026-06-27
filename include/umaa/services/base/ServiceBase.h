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

#ifndef INCLUDE_UMAA_SERVICES_BASE_SERVICEBASE_H_
#define INCLUDE_UMAA_SERVICES_BASE_SERVICEBASE_H_

#include <string>

#include "Logger.h"
#include "NumericGuid.h"

namespace arlcore::umaa::services {

class ServiceBase {
 public:
  explicit ServiceBase(
    const arlcore::NumericGuid& sourceId,
    const std::string& serviceName) :
      sourceId_(sourceId),
      serviceName_(serviceName) {}

  //! \brief Initialize resources required by the service
  virtual void startUp() {
    UMAA_LOG_DEBUG(util::SYSTEM_LOGGER, "StartUp function not overridden - using default implementation")
  }

  //! \brief Shutdown and cleanup resources used by the service
  virtual void shutDown() {
    UMAA_LOG_DEBUG(util::SYSTEM_LOGGER, "ShutDown function not overridden - using default implementation")
  }

  //! \brief Perform single execution of service cycle
  //! \return bool true for successful execution else false for service failure.
  virtual bool cycle() = 0;

  //! \brief Source ID of the service
  //! \return NumericGUID
  arlcore::NumericGuid getSourceId() const { return sourceId_; }

  //! \brief String name of the service
  //! \return string
  std::string getServiceName() const { return serviceName_; }

 private:
  const arlcore::NumericGuid sourceId_;
  const std::string serviceName_;
};

}  // namespace arlcore::umaa::services
#endif  // INCLUDE_UMAA_SERVICES_BASE_SERVICEBASE_H_
