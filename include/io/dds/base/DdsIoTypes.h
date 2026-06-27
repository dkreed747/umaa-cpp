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

#ifndef INCLUDE_IO_DDS_BASE_DDSIOTYPES_H_
#define INCLUDE_IO_DDS_BASE_DDSIOTYPES_H_

#include <cstdint>

namespace arlcore::io {

enum class SendStatus {
  SUCCESS,  // send successful
  ERROR,  // error in Sender
  NOT_IMPLEMENTED  // Enum to signify a function that returns SendStatus has not been implemented
};

enum class ReadStatus {
  SUCCESS,  // read successful
  ERROR,  // error in Reader
  NO_DATA,  // no data to read
  DISPOSED,  // sample disposed
  INVALID_DATA,  // implementation specific; data was read but not in valid state for use
  NOT_IMPLEMENTED  // Enum to signify a function that returns SendStatus has not been implemented
};

struct Duration {
  int64_t sec;
  uint64_t nanosec;
};
struct Time {
  int64_t sec;
  uint64_t nanosec;
};

template <class T>
struct SampleEnvelope {
  ReadStatus status;
  T data;
};

struct ReaderHealthStats {
  int32_t matchedSenders;
  int32_t rejectedSamples;
  int32_t lostSamples;
  int32_t missedDeadlines;
  int32_t incompatibleQos;
  int32_t invalidSamples;
  int32_t readSamples;
};

}  // namespace arlcore::io
#endif  // INCLUDE_IO_DDS_BASE_DDSIOTYPES_H_
