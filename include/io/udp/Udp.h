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
#ifndef INCLUDE_IO_UDP_UDP_H_
#define INCLUDE_IO_UDP_UDP_H_

#include <iostream>
#include <string>

namespace arlcore::io {

static const int MAX_UMSG_SIZE = 1500;

using UMSG = struct UMSG {
  int16_t len;  // length of message
  uint8_t data[MAX_UMSG_SIZE];  // data-bytes
};

/**
 * Header for UDP interface messages with length of 15 bytes
 */
struct UdpHeader {
  uint32_t msgID;  // 4 bytes
  uint16_t length;    // 2 bytes
  uint8_t checksum;  // 1 byte
  uint64_t timestamp;  // 8 bytes

  // Multiply by 2 because all header strings in UDP messages are listed as hex values (%02X)
  const uint8_t UdpHeaderLength = 2 * (sizeof(msgID) + sizeof(length) + sizeof(checksum) + sizeof(timestamp));
};

/**
 * Set of options specific to a UDP connection
 */
struct UdpOptions {
  uint16_t port;
  std::string ipAddr;
  bool multicast;
};
}  // namespace arlcore::io
#endif  // INCLUDE_IO_UDP_UDP_H_
