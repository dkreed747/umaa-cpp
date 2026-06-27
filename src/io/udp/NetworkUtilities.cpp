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

#include <arpa/inet.h>
#include <netdb.h>

#include "InternalTypes.h"
#include "NetworkUtilities.h"

namespace arlcore::io {

std::string NetworkUtilities::getIpAddressFromHostName(
    const std::string& hostname) {

  // Look up hostname and populate result if one is identified.
  struct addrinfo* info;

  std::string result;

  int32_t err = getaddrinfo(hostname.c_str(), nullptr, nullptr, &info);

  if (err == 0) {
    auto sa = (struct sockaddr_in*) info->ai_addr;

    result = inet_ntoa(sa->sin_addr);
  }

  return result;
}

}  // namespace arlcore::io







