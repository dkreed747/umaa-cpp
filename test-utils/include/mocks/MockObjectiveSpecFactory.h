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

#ifndef TEST_MOCKS_MOCKSENDABLESPECIALIZATION_H_
#define TEST_MOCKS_MOCKSENDABLESPECIALIZATION_H_

#include <gmock/gmock.h>

#include "ObjectiveSpecFactory.h"

using arlcore::umaa::SendableSpecializationBase;
using arlcore::umaa::ObjectiveSpecFactory;
using arlcore::umaa::DefaultSendableSpec;

namespace arlcore::test {
class MockObjectiveSpecFactory : public ObjectiveSpecFactory {
 public:
  MockObjectiveSpecFactory()
      : ObjectiveSpecFactory(make_shared<IOReaderRegistry>(), make_shared<IOWriterRegistry>()) {}
  MOCK_METHOD((std::shared_ptr<SendableSpecializationBase>), createSpecialization, (const ObjectiveType& genType), (override));
};

}  // namespace arlcore::test
#endif  // TEST_MOCKS_MOCKSENDABLESPECIALIZATION_H_