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

#ifndef INCLUDE_COMMON_GUID_H_
#define INCLUDE_COMMON_GUID_H_

#include <array>

// Should be compatible with UMAA type
using NumericGUID_t = std::array<uint8_t, 16L>;


#endif  // INCLUDE_COMMON_GUID_H_
