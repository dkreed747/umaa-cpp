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

#ifndef INCLUDE_UMAA_LARGELIST_H_
#define INCLUDE_UMAA_LARGELIST_H_

#include <list>
#include <map>
#include <memory>
#include <UMAA/Common/LargeListMetadata.hpp>
#include <UMAA/Common/Measurement/Measurements.hpp>

#include "Logger.h"
#include "NumericGuid.h"
#include "UmaaUtils.h"

namespace arlcore::umaa {

using UMAA::Common::LargeListMetadata;
using UMAA::Common::Measurement::DateTime;

enum class LargeListStatus { VALID_LIST, EMPTY_LIST, INVALID_LIST };

//! \brief A structure to hold the status of a list and a pointer to the result
//! \tparam Element The type of the elements contained in the list
template <class Element>
struct LargeListResult {
  LargeListStatus status;
  std::weak_ptr<const std::list<Element>> list;
};

//! \brief An internal representation of a Large List to be used by the LargeListReader class
//! \tparam ListElement The type of the element data this LargeList contains (e.g. GlobalWaypointType)
//! \tparam Element The type of the element message associated with the element data (e.g.
//!         GlobalWaypointCommandTypeWaypointsListElement)
template <class Element, class ListElement>
class LargeList {
 public:
  //! \brief Construct a Large List with a given ID
  //! \param id
  explicit LargeList(const NumericGuid &id) : listId_(id) {
    latest_metadata_.listID(id.getGuid());
    latest_metadata_.updateElementID(NIL_GUID.getGuid());
    latest_metadata_.startingElementID(NIL_GUID.getGuid());
    latest_metadata_.size(0);
  }
  //! \brief Construct a Large List from the provided metadata
  //! \param metadata
  explicit LargeList(const LargeListMetadata &metadata) : latest_metadata_(metadata), listId_(metadata.listID()) {}

  //! \brief Construct a list using the metadata and List Elements from this Large List
  //! \return A LargeListResult containing the state of the list and a pointer to the contents of the Large List
  //!         represented as a std::list of Elements
  LargeListResult<Element> getList() {
    LargeListResult<Element> result;
    if (cached_list_) {
      result.status = LargeListStatus::VALID_LIST;
      result.list = cached_list_;
      return result;
    } else if (latest_metadata_.size() == 0) {
      result.status = LargeListStatus::EMPTY_LIST;
      result.list = std::shared_ptr<std::list<Element>>(nullptr);
      return result;
    } else {
      return generateList();
    }
  }

  //! \brief Get the ID of this list
  //! \return The listID of this Large List as a NumericGuid
  const NumericGuid getId() const { return listId_; }

  //! \brief Handle a received ListElement sample
  //! \param element The ListElement being added/updated
  //! \return true if the ListElement was added/updated, false otherwise
  bool receive(const ListElement &element) {
    NumericGuid listId(element.listID());
    NumericGuid elementId(element.elementID());
    if (listId != listId_) {
      UMAA_LOG_ERROR(util::getLogger(), "Attempted to add a ListElement with the wrong listID " << listId);
      return false;
    }
    elementTable_[elementId] = element;
    return true;
  }

  //! \brief Handle a received LargeListMetadata sample
  //! \param metadata The metadata to update the Large List with
  //! \return A LargeListResult containing the state of the list and a pointer to the contents of the Large List
  //!         represented as a std::list of Elements
  LargeListResult<Element> receive(const LargeListMetadata &metadata) {
    LargeListResult<Element> result;
    result.list = std::shared_ptr<std::list<Element>>(nullptr);
    NumericGuid listId(metadata.listID());
    if (listId != listId_) {
      UMAA_LOG_ERROR(util::getLogger(), "Attempted to assign metadata with the wrong listID " << listId);
      result.status = LargeListStatus::INVALID_LIST;
      return result;
    }
    if (!metadata.updateElementTimestamp().has_value() || !latest_metadata_.updateElementTimestamp().has_value() ||
        metadata.updateElementTimestamp().value() >= latest_metadata_.updateElementTimestamp().value()) {
      latest_metadata_ = metadata;
      // Invalidate the currently cached list
      cached_list_ = std::shared_ptr<std::list<Element>>(nullptr);
    } else {
      UMAA_LOG_WARN(util::getLogger(), "Received metadata that is older than the current latest metadata");
      result.status = LargeListStatus::INVALID_LIST;
      return result;
    }
    return generateList();
  }

  //! \brief Handle a disposed instance of a ListElement
  //! \param element A ListElement with the key values of the disposed instance populated
  //! \return true if the ListElement was removed, false otherwise
  bool dispose(const ListElement element) {
    NumericGuid listId(element.listID());
    NumericGuid elementId(element.elementID());
    if (listId != listId_) {
      UMAA_LOG_ERROR(util::getLogger(), "Attempted to dispose a ListElement instance with the wrong listID " << listId);
      return false;
    }
    if (elementTable_.count(elementId) == 0) {
      UMAA_LOG_WARN(util::getLogger(), "Attempted to dispose a ListElement instance with unknown elementID "
                                          << elementId);
      return false;
    } else {
      elementTable_.erase(elementId);
    }
    return true;
  }

 private:
  NumericGuid listId_;
  LargeListMetadata latest_metadata_;
  std::map<NumericGuid, ListElement> elementTable_;
  std::shared_ptr<std::list<Element>> cached_list_;

  LargeListResult<Element> generateList() {
    LargeListResult<Element> result;
    result.list = std::shared_ptr<std::list<Element>>(nullptr);
    auto list = std::make_shared<std::list<Element>>();

    if (latest_metadata_.size() == 0) {
      result.status = LargeListStatus::EMPTY_LIST;
      return result;
    }

    std::map<NumericGuid, bool> visited;
    NumericGuid nextId(latest_metadata_.startingElementID());
    while (nextId != NIL_GUID) {
      auto element = elementTable_.find(nextId);
      if (element == elementTable_.end()) {
        UMAA_LOG_WARN(util::getLogger(), "No ListElement with ID " << nextId.getTwoDigitHexString());
        result.status = LargeListStatus::INVALID_LIST;
        return result;
      }
      visited[nextId] = true;
      auto e = element->second;
      if (latest_metadata_.updateElementTimestamp().has_value() &&
          e.elementTimestamp() > latest_metadata_.updateElementTimestamp().value()) {
        UMAA_LOG_WARN(util::getLogger(),
                     "ListElement ("
                         << nextId.getTwoDigitHexString()
                         << ") timestamp greater than metadata update timestamp, there may be an update in-progress");
        result.status = LargeListStatus::INVALID_LIST;
        return result;
      }

      list->push_back(e.element());
      if (e.nextElementID().has_value()) {
        nextId.setGuid(e.nextElementID().value());
        if (visited[nextId]) {
          UMAA_LOG_WARN(util::getLogger(), "Loop detected in Large List with ID " << listId_.getTwoDigitHexString());
          result.status = LargeListStatus::INVALID_LIST;
          return result;
        }
      } else {
        nextId.setGuid(NIL_GUID.getGuid());
      }
    }

    if (list->size() != latest_metadata_.size()) {
      result.status = LargeListStatus::INVALID_LIST;
      UMAA_LOG_WARN(util::getLogger(), "Generated list for (" << listId_.getTwoDigitHexString()
                                                             << ") size does not match size from Large List Metadata");
      return result;
    }

    result.list = list;
    result.status = LargeListStatus::VALID_LIST;
    cached_list_ = list;
    return result;
  }
};

}  // namespace arlcore::umaa

#endif  // INCLUDE_UMAA_LARGELIST_H_
