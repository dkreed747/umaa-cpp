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

#ifndef INCLUDE_UMAA_DOMAIN_UMAACOMMAND_H_
#define INCLUDE_UMAA_DOMAIN_UMAACOMMAND_H_

#include <UMAA/Common/IdentifierType.hpp>
#include "NumericGuid.h"

namespace arlcore::umaa::domain {

//! \brief Logical grouping that is the same as the UMAA base type UMAA::UMAACommand
//! Note: the sessionID does not need to be set for a command consumer since it is generated
struct CommandHeader {
  UMAA::Common::IdentifierType sourceId;
  UMAA::Common::IdentifierType destinationId;
  NumericGuid sessionId;

  bool operator==(const CommandHeader& other) const;
  bool operator!=(const CommandHeader& other) const;
};

inline bool CommandHeader::operator==(const CommandHeader &other) const {
  return sourceId == other.sourceId
      && destinationId == other.destinationId
      && sessionId == other.sessionId;
}

inline bool CommandHeader::operator!=(const CommandHeader &other) const {
  return !(*this == other);
}

}  // namespace arlcore::umaa::domain
#endif  // INCLUDE_UMAA_DOMAIN_UMAACOMMAND_H_
