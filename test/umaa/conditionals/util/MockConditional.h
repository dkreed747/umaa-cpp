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

#ifndef TEST_UMAA_CONDITIONALS_UTIL_MOCKCONDITIONAL_H_
#define TEST_UMAA_CONDITIONALS_UTIL_MOCKCONDITIONAL_H_

#include "ConditionalBase.h"

namespace arlcore::test {

class MockConditional : public arlcore::umaa::conditional::ConditionalBase {
 public:
  MockConditional(
    const UMAA::MM::Conditional::ConditionalType& conditional, bool evaluatesTo = false) :
    ConditionalBase(conditional), evaluatesTo_(evaluatesTo) {

  }
  
  void setEvaluationValue(bool value) {
    evaluatesTo_ = value;
  }

  std::optional<bool> evaluateConditional() const override {
    return evaluatesTo_;
  }

 private:
  bool evaluatesTo_;
};

}  // namespace arlcore::umaa::conditional
#endif  // TEST_UMAA_CONDITIONALS_UTIL_MOCKCONDITIONAL_H_
