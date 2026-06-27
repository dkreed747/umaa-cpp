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

#ifndef TEST_MOCKS_MOCKREADERSENDER_H_
#define TEST_MOCKS_MOCKREADERSENDER_H_

#include "gmock/gmock.h"

#include "ReaderBase.h"
#include "SenderBase.h"

namespace arlcore::test {
template <typename DataType>
class MockReaderSender : public arlcore::io::ReaderBase<DataType>,
                         public arlcore::io::SenderBase<DataType> {
 public:
  MockReaderSender() = default;
  MOCK_METHOD(SendStatus, send, (const DataType& data), (override));
  MOCK_METHOD(SendStatus, dispose, (const DataType& data), (override));
  MOCK_METHOD(ReadStatus, read, (DataType *outSample), (override));
  MOCK_METHOD(ReadStatus, readInstance, (const DataType& key, DataType *outSample), (override));
};
}  // namespace arlcore::test

#endif  // TEST_MOCKS_MOCKREADERSENDER_H_
