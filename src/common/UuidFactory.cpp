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

#include <uuid/uuid.h>

#include "UuidFactory.h"

namespace arlcore {

static const uint32_t UUID_STRING_SIZE = GUID_SIZE * 2 + 5;

NumericGuid UuidFactory::generateGuid() const {
  NumericGuid guid;
  std::array<uint8_t, GUID_SIZE> generatedGuid;

  // Use uuid to generate guids
  uuid_generate(&generatedGuid[0]);

  guid.setGuid(generatedGuid);

  return guid;
}

std::string UuidFactory::parseGuid(
    const std::array<uint8_t, GUID_SIZE> &inArray) const {
  // UUID String is 36 characters long, we double the bytes and add padding of 5
  std::array<char, UUID_STRING_SIZE> parsedGuid;

  uuid_unparse(&inArray[0], &parsedGuid[0]);
  std::string parsedGuidStr(&parsedGuid[0]);

  return parsedGuidStr;
}

std::string UuidFactory::parseGuid(const NumericGuid &rawGuid) const {
  return parseGuid(rawGuid.getGuid());
}

NumericGuid UuidFactory::parseGuidFromString(const std::string &rawGuid) const {
  std::array<uint8_t, GUID_SIZE> guidChar;
  uuid_parse(rawGuid.c_str(), &guidChar[0]);

  NumericGuid guid(guidChar);

  return guid;
}

}  // namespace arlcore

