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

#ifndef INCLUDE_CONFIG_CONFIGURATIONMANAGER_H_
#define INCLUDE_CONFIG_CONFIGURATIONMANAGER_H_

#include <yaml-cpp/yaml.h>
#include <string>

#include "AppConfig.h"

namespace arlcore {

class ConfigurationManager {
 public:
  ConfigurationManager() = default;
  virtual ~ConfigurationManager() = default;

  //! \brief Loads an application configuration from a file path.
  //!   The read in file will load the passed config member with data
  //! \param The file path of the configuration file to load.
  //! \param The configuration object to populate from the file.
  bool loadAppConfig(const std::string &filePath, AppConfig* config);

  //! \brief Load an application configuration and populate the internally
  //!   held config object.
  //! \param The file path of the configuration file to load.
  bool loadAppConfig(const std::string &filePath);

  //! \brief Get the Configuration Manager held  Configuration.
  const AppConfig& getAppConfig();

 private:
  AppConfig appConfig_;
};

}  // namespace arlcore

#endif  // INCLUDE_CONFIG_CONFIGURATIONMANAGER_H_

