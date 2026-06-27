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

#ifndef INCLUDE_UMAA_DESTINATIONREADERFILTER_H_
#define INCLUDE_UMAA_DESTINATIONREADERFILTER_H_

#include "NumericGuid.h"
#include "ReaderFilter.h"

namespace arlcore::umaa {

template <class DataType>
class DestinationReaderFilter : public arlcore::io::ReaderFilter<DataType> {
 public:
  explicit DestinationReaderFilter(NumericGuid id) : destinationId_(id) {}

  bool filter(const DataType& sample) override {
    return sample.destination().id() == destinationId_.getGuid();
  }

  void setFilterId(NumericGuid id) {
    destinationId_ = id;
  }

 private:
  NumericGuid destinationId_;
};

}  // namespace arlcore::umaa
#endif  // INCLUDE_UMAA_DESTINATIONREADERFILTER_H_
