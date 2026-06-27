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

#ifndef INCLUDE_IO_UDP_NETWORKUTILITIES_H_
#define INCLUDE_IO_UDP_NETWORKUTILITIES_H_

#include <string>

#include "NumericGuid.h"

namespace arlcore::io {

//! \brief A class used for Network Utility Functions
class NetworkUtilities {
 public:
  NetworkUtilities() = default;
  ~NetworkUtilities() = default;

  //! \brief Attempt to resolve the IP from the passed in hostname.
  //! \param The hostname to convert to an IP
  //! \return A string representation of an IP address.  This will be an empty
  //!    string if an IP is not resolved.
  static std::string getIpAddressFromHostName(const std::string& hostname);
};

}  // namespace arlcore::io

#endif  // INCLUDE_IO_UDP_NETWORKUTILITIES_H_

