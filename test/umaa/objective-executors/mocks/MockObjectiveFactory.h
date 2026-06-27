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

#ifndef  TEST_UMAA_OBJECTIVE_EXECUTORS_MOCKS_MOCKOBJECTIVEFACTORY_H_
#define TEST_UMAA_OBJECTIVE_EXECUTORS_MOCKS_MOCKOBJECTIVEFACTORY_H_

#include <memory>

#include "MockObjective.h"
#include "ObjectiveFactory.h"

namespace arlcore::test {

class MockObjectiveFactory : public arlcore::umaa::ObjectiveFactory {
 public:
  MockObjectiveFactory() :
    ObjectiveFactory(nullptr, nullptr, nullptr) {}

  std::optional<std::shared_ptr<arlcore::umaa::ObjectiveBase>>
    build(const arlcore::umaa::ObjectiveType& obj) const override {
      if (testBuildFail) {
        return std::nullopt;
      }

      auto mObj = std::optional<std::shared_ptr<MockObjective>>(std::make_shared<MockObjective>(
        obj,
        nullptr));

      if (testObjectiveValidationFail) {
        mObj.value()->isObjectiveValidFlag = false;
      }

      return mObj;
  }

  bool testBuildFail = true;
  bool testObjectiveValidationFail = false;
};

}  // namespace arlcore::test
#endif  // TEST_UMAA_OBJECTIVE_EXECUTORS_MOCKS_MOCKOBJECTIVEFACTORY_H_
