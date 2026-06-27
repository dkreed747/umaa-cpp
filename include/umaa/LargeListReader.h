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

#ifndef INCLUDE_UMAA_LARGELISTREADER_H_
#define INCLUDE_UMAA_LARGELISTREADER_H_

#include <list>
#include <map>
#include <memory>
#include <utility>
#include <UMAA/Common/LargeListMetadata.hpp>
#include <UMAA/Common/Measurement/Measurements.hpp>

#include "LargeList.h"
#include "Logger.h"
#include "NumericGuid.h"
#include "ReaderBase.h"

namespace arlcore::umaa {

using UMAA::Common::LargeListMetadata;
using UMAA::Common::Measurement::DateTime;

//! \brief An interface for reading and organizing data from LargeLists of a specific element type
//! \tparam Element The type of the element data this LargeList contains (e.g. GlobalWaypointType)
//! \tparam ListElement The type of the element message associated with the element data (e.g.
//!         GlobalWaypointCommandTypeWaypointsListElement)
template <class Element, class ListElement>
class LargeListReader {
 public:
  explicit LargeListReader(std::shared_ptr<io::ReaderBase<ListElement>> reader) : reader_(reader) {}

  //! \brief Return a list corresponding to the provided NumericGuid
  //! \param listId The ID of the list to retrieve
  //! \return A LargeListResult containing the state of the list and a pointer to the contents of the Large List
  //!         represented as a std::list of Elements
  LargeListResult<Element> getListById(const NumericGuid &listId) {
    LargeListResult<Element> result;
    updateListElements();
    auto list = listTable_.find(listId);
    if (list == listTable_.end()) {
      UMAA_LOG_WARN(util::getLogger(), "Attempting to get Large List with unknown ID " << listId)
      result.status = LargeListStatus::INVALID_LIST;
    } else {
      result = list->second.getList();
    }
    return result;
  }

  //! \brief Remove Large List by its listID. Should be called when a Large List is disposed.
  //! \param listId The ID of the list to remove as a NumericGuid
  //! \return true if the Large List was removed, false otherwise
  bool removeListById(const NumericGuid &listId) {
    updateListElements();
    if (listTable_.count(listId) == 0) {
      UMAA_LOG_WARN(util::SYSTEM_LOGGER,
                   "Attempting to remove Large List with unknown ID " << listId)
      return false;
    } else {
      listTable_.erase(listId);
      return true;
    }
  }

  //! \brief Return a list corresponding to the provided LargeListMetadata
  //! \param data The LargeListMetadata to construct the list from
  //! \return A LargeListResult containing the state of the list and a pointer to the contents of the Large List
  //!         represented as a std::list of Elements
  LargeListResult<Element> getListFromMetadata(const LargeListMetadata &data) {
    updateListElements();
    NumericGuid listId(data.listID());
    if (listTable_.count(listId) == 0) {
      listTable_.insert({listId, LargeList<Element, ListElement>(data)});
      return listTable_.at(listId).getList();
    } else {
      return listTable_.at(listId).receive(data);
    }
  }

  //! \brief Remove a Large List by its LargeListMetadata. Should be called when a Large List is disposed.
  //! \param data The LargeListMetadata corresponding to the disposed instance
  //! \return true if the Large List was removed, false otherwise
  bool removeListByMetadata(const LargeListMetadata &data) {
    return removeListById(NumericGuid(data.listID()));
  }

  //! \brief Read all ListElement samples available and process them. This function runs
  //!        when LargeListMetadata is processed but may also be run manually to keep the
  //!        LargeLists up to date.
  void updateListElements() {
    io::SampleEnvelope<ListElement> buffer[READ_BUFFER_SIZE];
    int totalReadCount = 0;
    // Read incoming samples in batches of up to READ_BUFFER_SIZE
    int readCount = reader_->readUpToN(buffer, READ_BUFFER_SIZE);
    while (readCount > 0) {
      for (int i = 0; i < readCount; i++) {
        // Log and ignore any invalid samples
        if (buffer[i].status == io::ReadStatus::INVALID_DATA) {
          UMAA_LOG_WARN(util::getLogger(), "Received invalid ListElement sample in LargeListReader");
          continue;
        }
        auto listId = NumericGuid(buffer[i].data.listID());
        auto elementId = NumericGuid(buffer[i].data.elementID());
        // If a new ListID appears, create an object to store in the list table
        if (listTable_.count(listId) == 0) {
          listTable_.insert({listId, LargeList<Element, ListElement>(listId)});
        }
        if (buffer[i].status == io::ReadStatus::DISPOSED) {
          // Attempt to dispose the sample from the corresponding set
          if (!listTable_.at(listId).dispose(buffer[i].data)) {
            UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Unable to dispose sample")
          }
        } else {
          // New sample. Add it to the corresponding list
          if (!listTable_.at(listId).receive(buffer[i].data)) {
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
  std::map<NumericGuid, LargeList<Element, ListElement>> listTable_;
  std::shared_ptr<io::ReaderBase<ListElement>> reader_;
  static const size_t READ_BUFFER_SIZE = 10;
  static const size_t MAX_READ_PER_UPDATE = 50;
};

}  // namespace arlcore::umaa

#endif  // INCLUDE_UMAA_LARGELISTREADER_H_
