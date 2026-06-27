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

#ifndef INCLUDE_COMMON_STRINGUTILS_H_
#define INCLUDE_COMMON_STRINGUTILS_H_

#include <utility>
#include <string>

namespace arlcore {

struct StringHash {
  using is_transparent = void;  // Enables heterogeneous operations.

  std::size_t operator()(const std::string_view sv) const {
    constexpr std::hash<std::string_view> hasher;
    return hasher(sv);
  }
};
}  // namespace arlcore

#endif  // INCLUDE_COMMON_STRINGUTILS_H_
