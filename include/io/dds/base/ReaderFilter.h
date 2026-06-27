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

#ifndef INCLUDE_IO_DDS_BASE_READERFILTER_H_
#define INCLUDE_IO_DDS_BASE_READERFILTER_H_

namespace arlcore::io {

template <class DataType>
class ReaderFilter {
 public:
  ReaderFilter() = default;
  virtual bool filter(const DataType& data) = 0;
};

}  // namespace arlcore::io
#endif  // INCLUDE_IO_DDS_BASE_READERFILTER_H_
