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

#ifndef INCLUDE_CONFIG_APPCONFIG_H_
#define INCLUDE_CONFIG_APPCONFIG_H_

#include <string>
#include "dds/dds.hpp"

#include "InternalTypes.h"
#include "NumericGuid.h"

namespace arlcore {

class AppConfig {
 public:
  AppConfig() = default;
  AppConfig(const int32_t& domainId, const std::string& qosFilePath,
      const std::string& domainQosProfile);
  AppConfig(const int32_t& domainId, const std::string& qosFilePath,
      const std::string& domainQosProfile,
      const std::string& largeCollectionsQosProfile);
  virtual ~AppConfig() = default;

  //! \brief Set the domain ID
  //! \param Domain id
  void setDomainId(const int32_t& id);
  //! \brief Get the Domain ID
  const int32_t& getDomainId() const;

  //! \brief Set the QOS file path
  //! \param QOS path
  void setQosFilePath(const std::string& path);
  //! \brief Get the QOS file path
  const std::string& getQosFilePath() const;

  //! \brief Set Domain Qos Profile
  //! \param profile
  void setDomainQosProfile(const std::string& profile);
  //! \brief Get the domain qos profile
  const std::string& getDomainQosProfile() const;

  //! \brief Set Large Collections Qos Profile
  //! \param Large Collections profile
  void setLargeCollectionsQosProfile(const std::string& profile);
  //! \brief Get the large collections qos profile
  const std::string& getLargeCollectionsQosProfile() const;

 private:
  int32_t domainId_ = 0;

  std::string qosFilePath_ = "";
  std::string domainQosProfile_ = "UMAA_QoS_Library::UMAA_Base_Profile";
  std::string largeCollectionsQosProfile_ = "UMAA_QoS_Library::UMAA_LargeCollections_Profile";
};

}  // namespace arlcore

#endif  // INCLUDE_CONFIG_APPCONFIG_H_
