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

#ifndef TEST_UTILS_INCLUDE_MOCKS_MOCKSENDABLESPECIALIZATION_H_
#define TEST_UTILS_INCLUDE_MOCKS_MOCKSENDABLESPECIALIZATION_H_

#include <gmock/gmock.h>

#include "SendableSpecializationBase.h"

using arlcore::umaa::SendableSpecializationBase;

namespace arlcore::test {

class MockSendableSpecialization : public SendableSpecializationBase {
 public:
  MOCK_METHOD(std::shared_ptr<SendableSpecializationBase>,
              with,
              (const NumericGUID& specID, const DateTime& timestamp),
              (const, override));
  MOCK_METHOD(SendStatus, send, (), (override));
  MOCK_METHOD(SendStatus, dispose, (), (override));
  MOCK_METHOD(NumericGUID, getSpecializationID, (), (override));
  MOCK_METHOD(DateTime, getSpecializationTimestamp, (), (override));
  MOCK_METHOD(std::string, getSpecializationTopic, (), (override));
};
}  // namespace arlcore::test
#endif  // TEST_UTILS_INCLUDE_MOCKS_MOCKSENDABLESPECIALIZATION_H_
