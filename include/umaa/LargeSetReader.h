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

#ifndef INCLUDE_UMAA_LARGESETREADER_H_
#define INCLUDE_UMAA_LARGESETREADER_H_

#include <map>
#include <memory>
#include <utility>
#include <UMAA/Common/LargeSetMetadata.hpp>
#include <UMAA/Common/Measurement/Measurements.hpp>

#include "LargeSet.h"
#include "Logger.h"
#include "NumericGuid.h"
#include "ReaderBase.h"

namespace arlcore::umaa {

using UMAA::Common::LargeSetMetadata;
using UMAA::Common::Measurement::DateTime;

//! \brief An interface for reading and organizing data from LargeSets of a specific element type
//! \tparam Element The type of the element data this LargeSet contains (e.g. GlobalWaypointType)
//! \tparam SetElement The type of the element message associated with the element data (e.g.
//!         GlobalWaypointCommandTypeWaypointsSetElement)
template <class Element, class SetElement>
class LargeSetReader {
 public:
  explicit LargeSetReader(std::shared_ptr<io::ReaderBase<SetElement>> reader) : reader_(reader) {}

  //! \brief Return a set corresponding to the provided NumericGuid
  //! \param setId The ID of the set to retrieve
  //! \return A LargeSetResult containing the state of the set and a pointer to the contents of the Large Set
  //!         represented as a std::set of Elements
  LargeSetResult<Element> getSetById(const NumericGuid &setId) {
    LargeSetResult<Element> result;
    updateSetElements();
    auto set = setTable_.find(setId);
    if (set == setTable_.end()) {
      UMAA_LOG_WARN(util::getLogger(), "Attempting to get Large Set with unknown ID " << setId)
      result.status = LargeSetStatus::INVALID_SET;
    } else {
      result = set->second.getSet();
    }
    return result;
  }

  //! \brief Remove a Large Set by its setID. Should be called when a Large Set is disposed.
  //! \param setId The ID of the set to remove as a NumericGuid
  //! \return true if the Large Set was removed, false otherwise
  bool removeSetById(const NumericGuid &setId) {
    updateSetElements();
    if (setTable_.count(setId) == 0) {
      UMAA_LOG_WARN(util::SYSTEM_LOGGER,
                   "Attempting to remove Large Set with unknown ID " << setId)
      return false;
    } else {
      setTable_.erase(setId);
      return true;
    }
  }

  //! \brief Return a set corresponding to the provided LargeSetMetadata
  //! \param data The LargeSetMetadata to construct the set from
  //! \return A LargeSetResult containing the state of the set and a pointer to the contents of the Large Set
  //!         represented as a std::set of Elements
  LargeSetResult<Element> getSetFromMetadata(const LargeSetMetadata &data) {
    updateSetElements();
    NumericGuid setId(data.setID());
    if (setTable_.count(setId) == 0) {
      setTable_.insert({setId, LargeSet<Element, SetElement>(data)});
      return setTable_.at(setId).getSet();
    } else {
      return setTable_.at(setId).receive(data);
    }
  }

  //! \brief Remove a Large Set by its LargeSetMetadata. Should be called when a Large Set is disposed.
  //! \param data The LargeSetMetadata corresponding to the Large Set to remove
  //! \return true if the Large Set was removed, false otherwise
  bool removeSetByMetadata(const LargeSetMetadata &data) {
    return removeSetById(NumericGuid(data.setID()));
  }

  //! \brief Read all SetElement samples available and process them. This function runs
  //!        when LargeSetMetadata is processed but may also be run manually to keep the
  //!        LargeSets up to date.
  void updateSetElements() {
    io::SampleEnvelope<SetElement> buffer[READ_BUFFER_SIZE];

    int totalReadCount = 0;
    // Read incoming samples in batches of up to READ_BUFFER_SIZE
    int readCount = reader_->readUpToN(buffer, READ_BUFFER_SIZE);
    while (readCount > 0) {
      for (int i = 0; i < readCount; i++) {
        // Log and ignore any invalid samples
        if (buffer[i].status == io::ReadStatus::INVALID_DATA) {
          UMAA_LOG_WARN(util::getLogger(), "Received invalid SetElement sample in LargeSetReader");
          continue;
        }
        auto setId = NumericGuid(buffer[i].data.setID());
        auto elementId = NumericGuid(buffer[i].data.elementID());
        // If a new SetID appears, create an object to store in the set table
        if (setTable_.count(setId) == 0) {
          setTable_.insert({setId, LargeSet<Element, SetElement>(setId)});
        }
        if (buffer[i].status == io::ReadStatus::DISPOSED) {
          // Attempt to dispose the sample from the corresponding set
          if (!setTable_.at(setId).dispose(buffer[i].data)) {
            UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Unable to dispose sample")
          }
        } else {
          // New sample. Add it to the corresponding set
          if (!setTable_.at(setId).receive(buffer[i].data)) {
            UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Unable to process incoming sample")
          }
        }
      }
      totalReadCount += readCount;
      if (totalReadCount > MAX_READ_PER_UPDATE) {
        // Don't read more if we've reached MAX_READ_PER_UPDATE
        readCount = 0;
      } else {
        readCount = reader_->readUpToN(buffer, READ_BUFFER_SIZE);
      }
    }
  }

 private:
  std::map<NumericGuid, LargeSet<Element, SetElement>> setTable_;
  std::shared_ptr<io::ReaderBase<SetElement>> reader_;
  static const size_t READ_BUFFER_SIZE = 10;
  static const size_t MAX_READ_PER_UPDATE = 50;
};

}  // namespace arlcore::umaa

#endif  // INCLUDE_UMAA_LARGESETREADER_H_
