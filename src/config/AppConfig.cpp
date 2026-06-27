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

#include "AppConfig.h"

namespace arlcore {

AppConfig::AppConfig(const int32_t &domainId, const std::string &qosFilePath,
    const std::string &domainQosProfile) :
    domainId_(domainId), qosFilePath_(qosFilePath), domainQosProfile_(
        domainQosProfile) {
}

AppConfig::AppConfig(const int32_t& domainId, const std::string& qosFilePath,
      const std::string& domainQosProfile,
      const std::string& largeCollectionsQosProfile) :
      domainId_(domainId), qosFilePath_(qosFilePath),
      domainQosProfile_(domainQosProfile),
      largeCollectionsQosProfile_(largeCollectionsQosProfile)  {
}

void AppConfig::setDomainId(const int32_t &id) {
  domainId_ = id;
}

const int32_t& AppConfig::getDomainId() const {
  return domainId_;
}

void AppConfig::setQosFilePath(const std::string &path) {
  qosFilePath_ = path;
}

const std::string& AppConfig::getQosFilePath() const {
  return qosFilePath_;
}

void AppConfig::setDomainQosProfile(const std::string &profile) {
  domainQosProfile_ = profile;
}

const std::string& AppConfig::getDomainQosProfile() const {
  return domainQosProfile_;
}

void AppConfig::setLargeCollectionsQosProfile(const std::string& profile) {
  largeCollectionsQosProfile_ = profile;
}

const std::string& AppConfig::getLargeCollectionsQosProfile() const {
  return largeCollectionsQosProfile_;
}

}  // namespace arlcore
