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

#ifndef INCLUDE_ENV_ENV_H_
#define INCLUDE_ENV_ENV_H_

#include <optional>
#include <string>

namespace arlcore::env {

//! @brief Helper function to load an environment variable by name and convert it to a passed type
//! @tparam T The type to load the variable as
//! @param variableName The name of the environment variable
//! @param variable The variable to load the data into if successful, passed as a non-const reference
//! @return boolean true if load was successful else false for failure
template <typename T>
static bool getEnv(const std::string& variableName, T& variable) {  // NOLINT - This needs to be a non-const reference
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

//! \brief Helper function to load an environment variable by name and convert it to passed type
//! \tparam T The type to load the variable as
//! \param variableName The name of the environment variable
//! \return returns optional wrapped type which can be checked to see if the load was successful
template <typename T>
static std::optional<T> getEnv(const std::string& variableName) {
  if (const char* value = std::getenv(variableName.c_str()); value != nullptr) {
    T returnValue;
    std::istringstream iss(value);
    if (iss >> returnValue) {
      return returnValue;
    }
  }

  return std::nullopt;
}

}  // namespace arlcore::env
#endif  // INCLUDE_ENV_ENV_H_
