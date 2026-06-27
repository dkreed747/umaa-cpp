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

#ifndef INCLUDE_COMMON_UUIDBASE_H_
#define INCLUDE_COMMON_UUIDBASE_H_

#include <string>

#include "NumericGuid.h"
#include "UuidFactory.h"

namespace arlcore {

class UuidBase {
 public:
  UuidBase() = default;
  virtual ~UuidBase() = default;

  const arlcore::NumericGuid& getId() const {
    return uuid_;
  }

  std::string getIdString() const {
    return arlcore::UuidFactory::getInstance().parseGuid(uuid_);
  }

 private:
  arlcore::NumericGuid uuid_ = arlcore::UuidFactory::getInstance().generateGuid();
};
}  // namespace arlcore
#endif  // INCLUDE_COMMON_UUIDBASE_H_
