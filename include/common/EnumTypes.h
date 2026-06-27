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

#ifndef INCLUDE_COMMON_ENUMTYPES_H_
#define INCLUDE_COMMON_ENUMTYPES_H_

#include <cstdint>

namespace arl {
  //! \brief Enumeration for UMAA Command Status Reasons
  enum class CommandStatus { CANCELED,
                              INTERRUPTED,
                              OBJECTIVE_FAILED,
                              RESOURCE_FAILED,
                              RESOURCE_REJECTED,
                              SERVICE_FAILED,
                              SUCCEEDED,
                              TIMEOUT,
                              UPDATED,
                              VALIDATION_FAILED };

  //! \brief Enumeration for UMAA Command Status States
  enum class CommandState { INITIAL_STATE,
                            ISSUED,
                            COMMANDED,
                            EXECUTING,
                            CANCELED,
                            COMPLETED,
                            FAILED,
                            INVALID };

  //! \brief Enumeration for Direction mode
  enum class DirectionModeEnum { COURSE,
                                  HEADING };
}  // namespace arl
#endif  // INCLUDE_COMMON_ENUMTYPES_H_
