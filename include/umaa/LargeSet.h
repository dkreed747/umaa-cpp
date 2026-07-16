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

#ifndef INCLUDE_UMAA_LARGESET_H_
#define INCLUDE_UMAA_LARGESET_H_

#include <unordered_set>
#include <map>
#include <memory>
#include <UMAA/Common/LargeSetMetadata.hpp>
#include <UMAA/Common/Measurement/Measurements.hpp>

#include "Logger.h"
#include "NumericGuid.h"
#include "UmaaUtils.h"

namespace arlcore::umaa {

using UMAA::Common::LargeSetMetadata;
using UMAA::Common::Measurement::DateTime;

enum class LargeSetStatus { VALID_SET, EMPTY_SET, INVALID_SET, STALE_METADATA };

//! \brief A structure to hold the status of a set and a pointer to the result
//! \tparam Element The type of the elements contained in the set
template <class Element>
struct LargeSetResult {
  LargeSetStatus status;
  std::weak_ptr<const std::unordered_set<Element, ElementHasher<Element>>> set;
};

//! \brief An internal representation of a Large Set to be used by the LargeSetReader class
//! \tparam Element The type of the element data this LargeSet contains (e.g. ObjectiveType)
//! \tparam SetElement The type of the element message associated with the element data (e.g.
//!         TaskPlanObjectivesSetElement)
template <class Element, class SetElement>
class LargeSet {
  using ElementSet = std::unordered_set<Element, ElementHasher<Element>>;
  using ElementSetPtr = std::shared_ptr<ElementSet>;

 public:
  //! \brief Construct a Large Set with a given ID
  //! \param id
  explicit LargeSet(const NumericGuid &id) : setId_(id) {
    latest_metadata_.setID(id.getGuid());
    latest_metadata_.updateElementID(NIL_GUID.getGuid());
    latest_metadata_.size(0);
  }
  //! \brief Construct a Large Set from the provided metadata
  //! \param metadata
  explicit LargeSet(const LargeSetMetadata &metadata) : latest_metadata_(metadata), setId_(metadata.setID()) {}

  //! \brief Construct a set using the metadata and Set Elements from this Large Set
  //! \return A LargeSetResult containing the state of the set and a pointer to the contents of the Large Set
  //!         represented as a std::unordered_set of Elements
  LargeSetResult<Element> getSet() {
    LargeSetResult<Element> result;
    if (cached_set_) {
      result.status = LargeSetStatus::VALID_SET;
      result.set = cached_set_;
      return result;
    } else if (latest_metadata_.size() == 0) {
      result.status = LargeSetStatus::EMPTY_SET;
      result.set = ElementSetPtr(nullptr);
      return result;
    } else {
      return generateSet();
    }
  }

  //! \brief Get the ID of this set
  //! \return The setID of this Large Set as a NumericGuid
  const NumericGuid getId() const { return setId_; }

  //! \brief Handle a received SetElement sample
  //! \param element The SetElement being added/updated
  //! \return true if the SetElement was added/updated, false otherwise
  bool receive(const SetElement &element) {
    NumericGuid setId(element.setID());
    NumericGuid elementId(element.elementID());
    if (setId != setId_) {
      UMAA_LOG_ERROR(util::getLogger(), "Attempted to add a SetElement with the wrong setID " << setId);
      return false;
    }
    elementTable_[elementId] = element;
    return true;
  }

  //! \brief Handle a received LargeSetMetadata sample
  //! \param metadata The metadata to update the Large Set with
  //! \return A LargeSetResult containing the state of the set and a pointer to the contents of the Large Set
  //!         represented as a std::unordered_set of Elements
  LargeSetResult<Element> receive(const LargeSetMetadata &metadata) {
    LargeSetResult<Element> result;
    result.set = ElementSetPtr(nullptr);
    NumericGuid setId(metadata.setID());
    if (setId != setId_) {
      UMAA_LOG_ERROR(util::getLogger(), "Attempted to assign metadata with the wrong setID " << setId);
      result.status = LargeSetStatus::INVALID_SET;
      return result;
    }
    if (!metadata.updateElementTimestamp().has_value() || !latest_metadata_.updateElementTimestamp().has_value() ||
        metadata.updateElementTimestamp().value() >= latest_metadata_.updateElementTimestamp().value()) {
      latest_metadata_ = metadata;
      // Invalidate the currently cached set
      cached_set_ = ElementSetPtr(nullptr);
    } else {
      UMAA_LOG_WARN(util::getLogger(), "Received metadata that is older than the current latest metadata");
      result.status = LargeSetStatus::STALE_METADATA;
      return result;
    }
    return generateSet();
  }

  //! \brief Handle a disposed instance of a SetElement
  //! \param element A SetElement with the key values of the disposed instance populated
  //! \return true if the SetElement was removed, false otherwise
  bool dispose(const SetElement element) {
    NumericGuid setId(element.setID());
    NumericGuid elementId(element.elementID());
    if (setId != setId_) {
      UMAA_LOG_ERROR(util::getLogger(), "Attempted to dispose a SetElement instance with the wrong setID " << setId);
      return false;
    }
    if (elementTable_.count(elementId) == 0) {
      UMAA_LOG_WARN(util::getLogger(),
                   "Attempted to dispose a SetElement instance with unknown elementID " << elementId);
      return false;
    } else {
      elementTable_.erase(elementId);
    }
    return true;
  }

  //! \brief Returns the number of unique SetElements (i.e. SetElements with the same ElementID) in the
  //!        LargeSet, i.e. std::distance(begin(), end()).
  //! \return The number of SetElements in the LargeSet
  size_t size() const { return elementTable_.size(); }

  //! \brief Checks if the LargeSet has no elements, i.e. whether begin() == end()
  //! \return true if the container is empty, false otherwise
  bool empty() const { return elementTable_.empty(); }

 private:
  NumericGuid setId_;
  LargeSetMetadata latest_metadata_;
  std::map<NumericGuid, SetElement> elementTable_;
  ElementSetPtr cached_set_;

  LargeSetResult<Element> generateSet() {
    LargeSetResult<Element> result;
    result.set = ElementSetPtr(nullptr);
    auto set = std::make_shared<ElementSet>();

    if (latest_metadata_.size() == 0) {
      result.status = LargeSetStatus::EMPTY_SET;
      return result;
    }

    for (const auto &[id, element] : elementTable_) {
      if (latest_metadata_.updateElementTimestamp().has_value() &&
          element.elementTimestamp() > latest_metadata_.updateElementTimestamp().value()) {
        UMAA_LOG_WARN(
            util::getLogger(),
            "SetElement (" << NumericGuid(element.setID())
                           << ") timestamp greater than metadata update timestamp, there may be an update in-progress");
        result.status = LargeSetStatus::INVALID_SET;
        return result;
      }
      set->insert(element.element());
    }

    if (set->size() != latest_metadata_.size()) {
      result.status = LargeSetStatus::INVALID_SET;
      UMAA_LOG_WARN(util::getLogger(),
                   "Generated set for (" << setId_ << ") size does not match size from Large Set Metadata");
      return result;
    }

    result.set = set;
    result.status = LargeSetStatus::VALID_SET;
    cached_set_ = set;
    return result;
  }
};

}  // namespace arlcore::umaa

#endif  // INCLUDE_UMAA_LARGESET_H_
