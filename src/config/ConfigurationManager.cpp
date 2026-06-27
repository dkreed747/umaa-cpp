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

#include <yaml-cpp/yaml.h>

#include <cstdlib>

#include "ConfigurationManager.h"
#include "Logger.h"
#include "UuidFactory.h"

namespace arlcore {

bool ConfigurationManager::loadAppConfig(const std::string &filePath,
    AppConfig* config) {
  bool fileLoaded = false;

  // Process YAML
  YAML::Node autonomyConfigYaml;

  try {
    autonomyConfigYaml = YAML::LoadFile(filePath);
    fileLoaded = true;
  } catch (YAML::BadFile &ex) {
    fileLoaded = false;
  }

  if (fileLoaded) {
    YAML::Node ddsConfig = autonomyConfigYaml["dds"];

    YAML::Node ddsDomainId = ddsConfig["domain-id"];
    if (ddsDomainId.IsDefined() && !ddsDomainId.IsNull()) {
      int32_t domainId = ddsDomainId.as<int32_t>();
      config->setDomainId(domainId);

      setenv("domain-id", std::to_string(domainId).c_str(), true);
    }

    YAML::Node ddsQosFile = ddsConfig["domain-qos-file"];
    if (ddsQosFile.IsDefined() && !ddsQosFile.IsNull()) {
      std::string domainQosFile = ddsQosFile.as<std::string>();
      config->setQosFilePath(domainQosFile);
      setenv("domain-qos-file", domainQosFile.c_str(), true);
    }

    YAML::Node ddsQosProfile = ddsConfig["domain-qos-profile"];
    if (ddsQosProfile.IsDefined() && !ddsQosProfile.IsNull()) {
      std::string qosProfile = ddsQosProfile.as<std::string>();
      config->setDomainQosProfile(qosProfile);

      setenv("domain-qos-profile", qosProfile.c_str(), true);
    }

    YAML::Node ddsLargeQosProfile = ddsConfig["large-collections-qos-profile"];
    if (ddsLargeQosProfile.IsDefined() && !ddsLargeQosProfile.IsNull()) {
      std::string largeCollectionsQosProfile = ddsLargeQosProfile.as<std::string>();
      config->setLargeCollectionsQosProfile(largeCollectionsQosProfile);

      setenv("large-collections-qos-profile", largeCollectionsQosProfile.c_str(), true);
    }
  }

  return fileLoaded;
}

bool ConfigurationManager::loadAppConfig(const std::string &filePath) {
  return loadAppConfig(filePath, &appConfig_);
}

const AppConfig& ConfigurationManager::getAppConfig() {
  return appConfig_;
}

}  // namespace arlcore
