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

#ifndef INCLUDE_COMMON_UUIDFACTORY_H_
#define INCLUDE_COMMON_UUIDFACTORY_H_

#include <string>

#include "NumericGuid.h"

namespace arlcore {
//! \brief A UUID/GUID Factory for generating, converting and parsing UUID/GUID
//!  representations into strings and vice versa
class UuidFactory {
 public:
  //! \brief Delete Copy constructor
  UuidFactory(UuidFactory const&) = delete;

  void operator=(UuidFactory const&) = delete;

  //! \brief Get the instance of the Factory singleton.
  //!    Will instantiate a static instance if one does not exist.
  static UuidFactory& getInstance() {
    static UuidFactory instance;

    return instance;
  }

  //! \brief Generates a GUID
  //! \return a generated GUID
  NumericGuid generateGuid() const;

  //! \brief Parse a GUID from a char array
  //! \param a GUID character array
  //! \return a string representation of the passed in GUID.
  std::string parseGuid(const std::array<uint8_t, GUID_SIZE> &inArray) const;

  //! \brief Parse a GUID from a known Numeric GUID type.
  //! \param A Numeric GUID type.
  //! \return a string representation of the passed in GUID.
  std::string parseGuid(const NumericGuid &rawGuid) const;

  //! \brief Convert a string representation of a GUID into a GUID type
  //! \param A GUID as a string
  //! \return a GUID type from the passed in string.
  NumericGuid parseGuidFromString(const std::string &rawGuid) const;

 private:
  UuidFactory() = default;
  virtual ~UuidFactory() = default;
};

}  // namespace arlcore

#endif  // INCLUDE_COMMON_UUIDFACTORY_H_

