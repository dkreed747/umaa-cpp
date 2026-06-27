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

#ifndef INCLUDE_IO_DDS_BASE_SENDERBASE_H_
#define INCLUDE_IO_DDS_BASE_SENDERBASE_H_

#include "DdsIoTypes.h"
#include "Logger.h"

namespace arlcore::io {

template <class DataType>
class SenderBase {
 public:
  //! \brief Got resources to clean up? Put that logic in your overridden destructor!
  //! OVERRIDE OPTIONAL
  virtual ~SenderBase() {}

  //! \brief send should be implemented to take the provided datatype and write it to the DDS bus
  //! Should be able to write keyed instances in addition to non-keyed samples
  //! OVERRIDE REQUIRED
  //! \param data the data to write to the bus
  //! \return SendStatus enum type
  virtual SendStatus send(const DataType& data) = 0;

  //! \brief waitForAcknowledgements should be implemented to block until the sender gets all acks from readers
  //! Note this will always return instantly unless a reliable dds qos policy is employed
  //! OVERRIDE OPTIONAL
  //! \param maxWaitDuration The maximum amount of time to block regardless of acks
  //! \return SendStatus enum
  virtual SendStatus waitForAcknowledgements(const Duration& maxWaitDuration) {
    UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Function not yet implemented.")
    return SendStatus::NOT_IMPLEMENTED;
  }

  //! \brief dispose should be implemented to change the state of an instance from alive to disposed
  //! OVERRIDE REQUIRED
  //! \param data the keyed datatype to dispose (Only the keyed data fields are required)
  //! \return SendStatus enum
  virtual SendStatus dispose(const DataType& data) = 0;
};

}  // namespace arlcore::io
#endif  // INCLUDE_IO_DDS_BASE_SENDERBASE_H_
