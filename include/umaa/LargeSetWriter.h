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

#ifndef INCLUDE_UMAA_LARGESETWRITER_H_
#define INCLUDE_UMAA_LARGESETWRITER_H_

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <UMAA/Common/LargeSetMetadata.hpp>
#include <UMAA/Common/Measurement/Measurements.hpp>

#include "LargeSet.h"
#include "Logger.h"
#include "NumericGuid.h"
#include "SenderBase.h"
#include "UmaaUtils.h"
#include "UuidFactory.h"

namespace arlcore::umaa {

using UMAA::Common::Measurement::DateTime;

//! \brief Interface for managing the writing of a UMAA LargeSet of elements
//! \tparam Element The type of the element data this LargeSet contains (e.g. ObjectiveType)
//! \tparam SetElement The type of the element message associated with the element data (e.g.
//!         TaskPlanObjectivesSetElement)
template <class Element, class SetElement>
class LargeSetWriter {
 public:
  // Delete default constructor
  LargeSetWriter() = delete;

  //! \brief Constructor
  //! \param sender An instance of the DDS sender interface that writes to the SetElement topic
  explicit LargeSetWriter(std::shared_ptr<io::SenderBase<SetElement>> sender) : elementSender_(sender) {
    setId_ = UuidFactory::getInstance().generateGuid();
    metadata_.setID(setId_.getGuid());
    metadata_.size(0);
    metadata_.updateElementID(NIL_GUID.getGuid());
  }

  //! \brief Constructor
  //! \param sender An instance of the DDS sender interface that writes to the SetElement topic
  //! \param elements An ordered vector of elements to write to the LargeSet
  LargeSetWriter(std::shared_ptr<io::SenderBase<SetElement>> sender, const std::vector<Element>& elements)
      : elementSender_(sender) {
    setId_ = UuidFactory::getInstance().generateGuid();
    metadata_.setID(setId_.getGuid());
    metadata_.size(0);
    metadata_.updateElementID(NIL_GUID.getGuid());
    // We prepend the set of elements in reverse because it has to update less samples (doesn't have to update
    // nextElementID)
    for (auto it = elements.cbegin(); it != elements.cend(); it++) {
      insert(*it);
    }
  }

  //! \brief Destructor - cleans up by disposing any remaining SetElement instances
  ~LargeSetWriter() {
    this->clear();
  }

  //! \brief Update an existing element in the LargeSet
  //! \param prev The element to update
  //! \param value The new value of the element
  //! \return The overall status returned from sending the DDS messages required to perform this operation
  io::SendStatus update(const Element& prev, const Element& value) {
    auto pos = elementTable_.find(prev);
    if (pos == elementTable_.end()) {
      UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Attempted to update Element that does not exist in set.");
      return io::SendStatus::ERROR;
    }
    SetElement e = pos->second;
    e.element(value);
    e.elementTimestamp(getTimestamp());
    if (elementSender_->send(e) == io::SendStatus::ERROR) {
      UMAA_LOG_ERROR(util::getLogger(), "Unable to update element in large set.");
      return io::SendStatus::ERROR;
    }
    if (elements_.erase(pos->second) != 1) {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Failed to remove set old entry during update");
    }
    elements_.insert(e);
    elementTable_.erase(pos);
    elementTable_.insert(std::make_pair(value, e));

    updateMetadata(e);
    return io::SendStatus::SUCCESS;
  }

  //! \brief Update an existing element in the LargeSet from its element ID
  //! \param elementId The element ID of the set element to update
  //! \param value The new value of the element
  //! \return The overall status returned from sending the DDS messages required to perform this operation
  io::SendStatus update(const NumericGuid& elementId, const Element& value) {
    if (std::optional<Element> e = getElementById(elementId); e.has_value()) {
      return update(e.value(), value);
    } else {
      UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Unable to find element with ID " << elementId << " to update")
      return io::SendStatus::ERROR;
    }
  }

  //! \brief Add an element to the Large Set
  //! \param value The element to add
  //! \param elementId A pointer to the element ID to set for the element to or output the generated ID to
  //! \param generateId Whether to generate an ID for the element or use one provided in the next argument
  //! \return The overall status returned from sending the DDS messages required to perform this operation
  io::SendStatus insert(const Element& value, NumericGuid *elementId = nullptr, bool generateId = true) {
    if (elementTable_.count(value)) {
      UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Attempted to add Element that already exists in set.");
      return io::SendStatus::ERROR;
    } else if (!generateId && elementId == nullptr) {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Attempted to set an element ID of an inserted element with a nullptr");
      return io::SendStatus::ERROR;
    }
    SetElement e = newSetElement(value);
    if (elementId != nullptr) {
      if (generateId) {
        *elementId = NumericGuid(e.elementID());
      } else {
        e.elementID(elementId->getGuid());
      }
    }

    if (elementSender_->send(e) == io::SendStatus::ERROR) {
      UMAA_LOG_ERROR(util::getLogger(), "Unable to add element to large set.");
      return io::SendStatus::ERROR;
    }
    elementTable_[value] = e;
    elements_.insert(e);
    updateMetadata(e);

    return io::SendStatus::SUCCESS;
  }

  //! \brief Add elements to the Large Set
  //! \param values The elements to add
  //! \param elementIds A pointer to the vector of element IDs to set elements to or the output generated IDs to
  //! \param generateId Whether to generate an ID for the element or use one provided in the next argument
  //! \return The overall status returned from sending the DDS messages required to perform this operation
  io::SendStatus insert(const std::vector<Element>& values, std::vector<NumericGuid> *elementIds = nullptr,
      bool generateIds = true) {
    if (values.size() == 0) {
      UMAA_LOG_ERROR(util::getLogger(), "Attempted to add empty vector to large set.");
      return io::SendStatus::ERROR;
    }
    if (!generateIds) {
      if (elementIds == nullptr) {
        UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Attempted to set element IDs of inserted elements from a nullptr");
        return io::SendStatus::ERROR;
      } else if (elementIds->size() != values.size()) {
        UMAA_LOG_ERROR(util::SYSTEM_LOGGER,
            "Attempted to set the element ID of inserted elements from a vector of mismatched length");
        return io::SendStatus::ERROR;
      }
    }

    for (auto it = values.begin(); it != values.end(); ++it) {
      if (elementTable_.count(*it)) {
        UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Attempted to add Element that already exists in set.");
        return io::SendStatus::ERROR;
      }
      SetElement e = newSetElement(*it);
      if (elementIds != nullptr) {
        if (generateIds) {
          elementIds->emplace_back(e.elementID());
        } else {
          e.elementID(elementIds->at(std::distance(values.begin(), it)).getGuid());
        }
      }

      if (elementSender_->send(e) == io::SendStatus::ERROR) {
        UMAA_LOG_ERROR(util::getLogger(), "Unable to add element to large set.");
        return io::SendStatus::ERROR;
      }
      elementTable_[*it] = e;
      elements_.insert(e);
      updateMetadata(e);
    }

    return io::SendStatus::SUCCESS;
  }

  //! \brief Remove an element from the LargeSet with a given value
  //! \param value The element to be removed
  //! \return The overall status returned from sending the DDS messages required to perform this operation
  io::SendStatus remove(const Element& value) {
    auto remove = elementTable_.find(value);
    if (remove == elementTable_.end()) {
      UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Attempted to remove Element that does not exist in set.");
      return io::SendStatus::ERROR;
    }
    auto removed = remove->second;
    if (elementSender_->dispose(removed) == io::SendStatus::ERROR) {
      UMAA_LOG_ERROR(util::getLogger(), "Unable to remove element from large set.");
      return io::SendStatus::ERROR;
    }
    elementTable_.erase(value);
    elements_.erase(removed);

    updateMetadata(removed, true);

    return io::SendStatus::SUCCESS;
  }

  //! \brief Remove an element from the Large Set with a given element ID
  //! \param elementId The ID of the element to remove from the set
  //! \return The overall status returned from sending the DDS messages required to perform this operation
  io::SendStatus remove(const NumericGuid& elementId) {
    if (std::optional<Element> e = getElementById(elementId); e.has_value()) {
      return remove(e.value());
    } else {
      UMAA_LOG_WARN(util::SYSTEM_LOGGER, "Unable to find element with ID " << elementId << " to remove");
      return io::SendStatus::ERROR;
    }
  }

  //! \brief Clears the LargeSet by disposing all of its elements
  //! \return The overall status returned from sending the DDS messages required to perform this operation
  io::SendStatus clear() {
    io::SendStatus status = io::SendStatus::SUCCESS;
    // Calling remove invalidates the iterator so first we generate a local copy of the elements
    std::vector<Element> elements;
    for (auto elem = elementTable_.begin(); elem != elementTable_.end(); ++elem) {
      elements.push_back(elem->first);
    }
    // Now we remove all of the elements from our local copy
    for (auto elem : elements) {
      if (remove(elem) != io::SendStatus::SUCCESS) {
        status = io::SendStatus::ERROR;
      }
    }

    return status;
  }

  //! \brief Retrieve an Element in the Large Set by its element ID
  //! \param elementId The element ID of the element to retrieve
  //! \return The element with the ID or nullopt if not found
  std::optional<Element> getElementById(const NumericGuid& elementId) {
    auto it = std::find_if(elementTable_.begin(), elementTable_.end(), [&elementId] (std::pair<Element, SetElement> e) {
      if (e.second.elementID() == elementId.getGuid()) {
        return true;
      }
      return false;
    });
    return it != elementTable_.end() ? std::optional<Element>(it->first) : std::nullopt;
  }

  //! \brief A function to find a SetElement in the SetTable matching an Element condition.
  //! \param condition A lambda condition to match with the Element
  //! \return The stored SetElement (containing the Element) that matches the condition
  std::optional<SetElement> findSetElementIf(const std::function<bool(const Element&)>& condition) {
    auto it = std::find_if(elementTable_.begin(), elementTable_.end(),
                            [&condition](const auto& pair) {
                              return condition(pair.first); });
    return it != elementTable_.end() ? std::make_optional<SetElement>(it->second) : std::nullopt;
  }

  //! \brief Get the metadata corresponding to the working state of the LargeSet, potentially with pending operations
  //! \return The LargeSet metadata object
  const UMAA::Common::LargeSetMetadata getMetadata() const { return metadata_; }

  //! \brief Get the ID corresponding to the LargeSet associated with this writer
  //! \return A NumericGuid representation of the setID
  const NumericGuid getSetId() const { return setId_; }

  //! \brief Returns a const iterator to the first SetElement of the LargeSet
  //!        If the LargeSet is empty, the returned iterator will be equal to \ref end()
  //! \return const iterator to the first SetElement
  typename std::unordered_set<SetElement, SetElementHasher<SetElement>>::const_iterator begin() const noexcept {
    return elements_.begin();
  }

  //! \brief Returns a const iterator to the first SetElement of the LargeSet
  //!        If the LargeSet is empty, the returned iterator will be equal to \ref cend()
  //! \return const iterator to the first SetElement
  typename std::unordered_set<SetElement, SetElementHasher<SetElement>>::const_iterator cbegin() const noexcept {
    return elements_.cbegin();
  }

  //! \brief Returns a const iterator to the element following the last SetElement of the LargeSet
  //!        This element acts as a placeholder; attempting to access it results in undefined behavior
  //! \return const iterator to the element following the last SetElement
  typename std::unordered_set<SetElement, SetElementHasher<SetElement>>::const_iterator end() const noexcept {
    return elements_.end();
  }

  //! \brief Returns a const iterator to the element following the last SetElement of the LargeSet
  //!        This element acts as a placeholder; attempting to access it results in undefined behavior
  //! \return const iterator to the element following the last SetElement
  typename std::unordered_set<SetElement, SetElementHasher<SetElement>>::const_iterator cend() const noexcept {
    return elements_.cend();
  }

  //! \brief Checks if the LargeSet has no elements, i.e. whether begin() == end()
  //! \return true if the container is empty, false otherwise
  bool empty() const noexcept {
    return elements_.empty();
  }

  //! \brief Returns the number of unique SetElements (i.e. SetElements with the same ElementID) in the
  //!        LargeSet, i.e. std::distance(begin(), end()).
  //! \return The number of SetElements in the LargeSet
  std::size_t size() const noexcept {
    return elements_.size();
  }

  //! \brief Returns the maximum number of SetElements that the underlying container is able to hold due to
  //!        system or library implementation limitations, i.e. std::distance(begin(), end()) for the largest container.
  //! \return Maximum number of elements
  std::size_t max_size() const noexcept {
    return elements_.max_size();
  }

 private:
  std::unordered_set<SetElement, SetElementHasher<SetElement>> elements_;
  std::unordered_map<Element, SetElement, ElementHasher<Element>> elementTable_;
  std::shared_ptr<io::SenderBase<SetElement>> elementSender_;
  UMAA::Common::LargeSetMetadata metadata_;
  NumericGuid setId_;

  void updateMetadata(const SetElement& e, bool dispose = false) {
    metadata_.updateElementID(e.elementID());
    if (dispose) {
      metadata_.updateElementTimestamp().reset();
    } else {
      metadata_.updateElementTimestamp(e.elementTimestamp());
    }
    auto size = elements_.size();
    metadata_.size(size);
  }

  SetElement newSetElement(const Element& value) {
    auto newElementId = UuidFactory::getInstance().generateGuid().getGuid();
    SetElement e;
    e.element(value);
    e.setID(setId_.getGuid());
    e.elementID(newElementId);
    e.elementTimestamp(getTimestamp());
    return e;
  }
};

}  // namespace arlcore::umaa

#endif  // INCLUDE_UMAA_LARGESETWRITER_H_
