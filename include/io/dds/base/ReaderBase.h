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

#ifndef INCLUDE_IO_DDS_BASE_READERBASE_H_
#define INCLUDE_IO_DDS_BASE_READERBASE_H_

#include <memory>

#include "DdsIoTypes.h"
#include "Logger.h"
#include "ReaderFilter.h"

namespace arlcore::io {

template <class DataType>
class ReaderBase {
 public:
  //! \brief Got resources to clean up? Put that logic in your overridden destructor!
  //! OVERRIDE OPTIONAL
  virtual ~ReaderBase() {}

  //! \brief read should be implemented to read the NEXT sample off the bus
  //! OVERRIDE REQUIRED
  //! \param outSample out variable to write any retrieved data to
  //! \return ReadsStatus enum
  virtual ReadStatus read(DataType *outSample) = 0;

  //! \brief readLatest should be implemented to read the LATEST sample off the bus
  //! OVERRIDE OPTIONAL
  //! \param outSample out variable to write any retrieved data to
  //! \return ReadStatus enum
  virtual ReadStatus readLatest(DataType *outSample) {
    UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Function not implemented!")
    return ReadStatus::NOT_IMPLEMENTED;
  }

  //! \brief readUpToN should be implemented to return a maximum number of samples and
  //! return them in sample envelope wrappers
  //! OVERRIDE OPTIONAL
  //! \param outSamples A buffer of envelope types
  //! \param n The max number of samples to read
  //! \return the number of samples actually read off the bus
  virtual size_t readUpToN(SampleEnvelope<DataType>* outSamples, const uint32_t& n) {
    UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Function not implemented!")
    return 0;
  }

  //! \brief readInstance should be implemented to read a KEYED instance sample
  //! OVERRIDE OPTIONAL
  //! \param key a sample with the keyed fields populated
  //! \param outSample out variable to write any retrieved data to
  virtual ReadStatus readInstance(const DataType& key, DataType *outSample) {
    UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Function not implemented!")
    return ReadStatus::NOT_IMPLEMENTED;
  }

  //! \brief getHealthInfo should be implemented to populate a ReaderHealthStats type
  //! OVERRIDE OPTIONAL
  //! using information provided by the vendor
  //! \return ReaderHealthStats object
  virtual ReaderHealthStats getHealthInfo() {
    UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Function not implemented!")
    return ReaderHealthStats();
  }

  //! \brief Specify a function to filter incoming samples, nullptr to disable
  //! \param filter The function to call to determine if a sample is accepted
  void setManualFilter(std::weak_ptr<ReaderFilter<DataType>> filter) {
    manualFilter = filter;
  }

 protected:
  std::weak_ptr<ReaderFilter<DataType>> manualFilter;
};

}  // namespace arlcore::io
#endif  // INCLUDE_IO_DDS_BASE_READERBASE_H_
