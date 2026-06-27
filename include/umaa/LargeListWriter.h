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

#ifndef INCLUDE_UMAA_LARGELISTWRITER_H_
#define INCLUDE_UMAA_LARGELISTWRITER_H_

#include <list>
#include <memory>
#include <vector>
#include <UMAA/Common/LargeListMetadata.hpp>
#include <UMAA/Common/Measurement/Measurements.hpp>

#include "Logger.h"
#include "NumericGuid.h"
#include "RealtimeSystemClock.h"
#include "SenderBase.h"
#include "UmaaUtils.h"
#include "UuidFactory.h"

namespace arlcore::umaa {

using UMAA::Common::Measurement::DateTime;

//! \brief Interface for managing the writing of a UMAA LargeList of elements
//! \tparam Element The type of the element data this LargeList contains (e.g. GlobalWaypointType)
//! \tparam ListElement The type of the element message associated with the element data (e.g.
//! GlobalWaypointCommandTypeWaypointsListElement)
template <class Element, class ListElement>
class LargeListWriter {
 public:
  // Delete default constructor
  LargeListWriter() = delete;

  //! \brief Constructor
  //! \param sender An instance of the DDS sender interface that writes to the ListElement topic
  explicit LargeListWriter(std::shared_ptr<io::SenderBase<ListElement>> sender) : elementSender_(sender) {
    listId_ = UuidFactory::getInstance().generateGuid();
    metadata_.listID(listId_.getGuid());
    metadata_.size(0);
    metadata_.updateElementID(NIL_GUID.getGuid());
    metadata_.startingElementID(NIL_GUID.getGuid());
  }

  //! \brief Constructor
  //! \param sender An instance of the DDS sender interface that writes to the ListElement topic
  //! \param elements An ordered vector of elements to write to the LargeList
  LargeListWriter(std::shared_ptr<io::SenderBase<ListElement>> sender, const std::vector<Element>& elements)
      : elementSender_(sender) {
    listId_ = UuidFactory::getInstance().generateGuid();
    metadata_.listID(listId_.getGuid());
    metadata_.size(0);
    metadata_.updateElementID(NIL_GUID.getGuid());
    // We prepend the list of elements in reverse because it has to update less samples (doesn't have to update
    // nextElementID)
    prepend(elements);
  }

  //! \brief Destructor - cleans up by disposing any remaining ListElement instances
  ~LargeListWriter() {
    this->clear();
  }

  //! \brief Update an existing element in the LargeList
  //! \param value The updated element
  //! \param pos The position of the element to update
  //! \return The overall status returned from sending the DDS messages required to perform this operation
  io::SendStatus update(const Element& value, uint64_t pos) {
    if (pos >= elements_.size()) {
      return io::SendStatus::ERROR;
    }
    auto it = elements_.begin();
    std::advance(it, pos);
    ListElement e = *it;
    e.element(value);
    e.elementTimestamp(getTimestamp());
    if (elementSender_->send(e) == io::SendStatus::ERROR) {
      return io::SendStatus::ERROR;
    }
    *it = e;

    updateMetadata(e);
    return io::SendStatus::SUCCESS;
  }

  //! \brief Add an element to the beginning of the LargeList
  //! \param value The element to add
  //! \return The overall status returned from sending the DDS messages required to perform this operation
  io::SendStatus prepend(const Element& value) {
    ListElement e = newListElement(value);
    if (elements_.size() != 0) {
      e.nextElementID(elements_.front().elementID());
    }

    e.elementTimestamp(getTimestamp());
    if (elementSender_->send(e) == io::SendStatus::ERROR) {
      return io::SendStatus::ERROR;
    }
    elements_.push_front(e);

    updateMetadata(e);

    return io::SendStatus::SUCCESS;
  }

  //! \brief Add an ordered container of elements to the beginning of the LargeList
  //! \param values The ordered container of elements to add
  //! \return The overall status returned from sending the DDS messages required to perform this operation
  io::SendStatus prepend(const std::vector<Element>& values) {
    if (values.size() == 0) {
      UMAA_LOG_ERROR(util::getLogger(), "Attempted to add empty vector to large list.");
      return io::SendStatus::ERROR;
    } else if (values.size() == 1) {
      return prepend(values.front());
    }
    for (auto it = values.rbegin(); it != values.rend(); ++it) {
      ListElement e = newListElement(*it);
      if (elements_.size() != 0) {
        e.nextElementID(elements_.front().elementID());
      }
      e.elementTimestamp(getTimestamp());
      if (elementSender_->send(e) == io::SendStatus::ERROR) {
        UMAA_LOG_ERROR(util::getLogger(), "Unable to add element to large list.")
        return io::SendStatus::ERROR;
      }
      elements_.push_front(e);

      updateMetadata(e);
    }
    return io::SendStatus::SUCCESS;
  }

  //! \brief Add an element to the end of the LargeList
  //! \param value The element to add
  //! \return The overall status returned from sending the DDS messages required to perform this operation
  io::SendStatus append(const Element& value) {
    ListElement e = newListElement(value);
    if (elements_.size() != 0) {
      elements_.back().nextElementID(e.elementID());
      elements_.back().elementTimestamp(getTimestamp());
      if (elementSender_->send(elements_.back()) == io::SendStatus::ERROR) {
        return io::SendStatus::ERROR;
      }
    }

    e.elementTimestamp(getTimestamp());
    if (elementSender_->send(e) == io::SendStatus::ERROR) {
      UMAA_LOG_ERROR(util::getLogger(), "Unable to add element to large list. List is likely no longer valid.");
      return io::SendStatus::ERROR;
    }
    elements_.push_back(e);

    updateMetadata(e);

    return io::SendStatus::SUCCESS;
  }

  //! \brief Add an ordered container of elements to the end of the LargeList
  //! \param values The ordered container of elements to add
  //! \return The overall status returned from sending the DDS messages required to perform this operation
  io::SendStatus append(const std::vector<Element>& values) {
    if (values.size() == 0) {
      UMAA_LOG_ERROR(util::getLogger(), "Attempted to add empty vector to large list.");
      return io::SendStatus::ERROR;
    } else if (values.size() == 1) {
      return append(values.front());
    }
    std::vector<ListElement> newElements;
    for (auto it = values.rbegin(); it != values.rend(); ++it) {
      ListElement e = newListElement(*it);
      if (it != values.rbegin()) {
        e.nextElementID(newElements.back().elementID());
      }
      newElements.push_back(e);
    }

    // Update last element to point to first element of new elements
    elements_.back().nextElementID(newElements.back().elementID());
    elements_.back().elementTimestamp(getTimestamp());
    if (elementSender_->send(elements_.back()) == io::SendStatus::ERROR) {
      UMAA_LOG_ERROR(util::getLogger(), "Unable to update element in large list.");
      return io::SendStatus::ERROR;
    }

    // Publish newElements in order (reverse of reverse)
    for (auto it = newElements.rbegin(); it != newElements.rend(); ++it) {
      it->elementTimestamp(getTimestamp());
      if (elementSender_->send(*it) == io::SendStatus::ERROR) {
        UMAA_LOG_ERROR(util::getLogger(), "Unable to add element to large list. List is likely no longer valid.");
        return io::SendStatus::ERROR;
      }
      elements_.push_back(*it);
    }

    updateMetadata(elements_.back());

    return io::SendStatus::SUCCESS;
  }

  //! \brief Add an element to the LargeList at a specific index
  //! \param value The element to add
  //! \param pos The position in the list to add the new element
  //! \return The overall status returned from sending the DDS messages required to perform this operation
  io::SendStatus insert(const Element& value, uint64_t pos) {
    if (pos > elements_.size()) {
      return io::SendStatus::ERROR;
    } else if (pos == elements_.size()) {
      return append(value);
    } else if (pos == 0) {
      return prepend(value);
    }

    ListElement e = newListElement(value);

    auto it = elements_.begin();
    std::advance(it, pos - 1);
    it->nextElementID(e.elementID());
    it->elementTimestamp(getTimestamp());
    if (elementSender_->send(*it) == io::SendStatus::ERROR) {
      return io::SendStatus::ERROR;
    }

    e.nextElementID(std::next(it)->elementID());
    e.elementTimestamp(getTimestamp());
    if (elementSender_->send(e) == io::SendStatus::ERROR) {
      UMAA_LOG_ERROR(util::getLogger(), "Unable to add element to large list. List is likely no longer valid.");
      return io::SendStatus::ERROR;
    }
    elements_.insert(it, e);

    updateMetadata(e);

    return io::SendStatus::SUCCESS;
  }

  //! \brief Add an element to the LargeList at a specific index
  //! \param value The container of elements to add
  //! \param pos The position in the list to add the new elements
  //! \return The overall status returned from sending the DDS messages required to perform this operation
  io::SendStatus insert(const std::vector<Element>& values, uint64_t pos) {
    if (values.size() == 0) {
      UMAA_LOG_ERROR(util::getLogger(), "Attempted to add empty vector to large list.");
      return io::SendStatus::ERROR;
    } else if (values.size() == 1) {
      return insert(values.front(), pos);
    }

    if (pos > elements_.size()) {
      return io::SendStatus::ERROR;
    } else if (pos == elements_.size()) {
      return append(values);
    } else if (pos == 0) {
      return prepend(values);
    }

    std::vector<ListElement> newElements;
    for (auto it = values.rbegin(); it != values.rend(); ++it) {
      ListElement e = newListElement(*it);
      if (it != values.rbegin()) {
        e.nextElementID(newElements.back().elementID());
      }
      newElements.push_back(e);
    }

    auto it = elements_.begin();
    std::advance(it, pos - 1);
    it->nextElementID(newElements.back().elementID());
    it->elementTimestamp(getTimestamp());
    if (elementSender_->send(*it) == io::SendStatus::ERROR) {
      UMAA_LOG_ERROR(util::getLogger(), "Unable to update element in large list.");
      return io::SendStatus::ERROR;
    }

    newElements.front().nextElementID(std::next(it)->elementID());

    // Publish newElements in order (reverse of reverse)
    for (auto it = newElements.rbegin(); it != newElements.rend(); ++it) {
      it->elementTimestamp(getTimestamp());
      if (elementSender_->send(*it) == io::SendStatus::ERROR) {
        UMAA_LOG_ERROR(util::getLogger(), "Unable to add element to large list. List is likely no longer valid.");
        return io::SendStatus::ERROR;
      }
    }
    elements_.insert(++it, newElements.begin(), newElements.end());

    updateMetadata(newElements.front());

    return io::SendStatus::SUCCESS;
  }

  //! \brief Remove the last element from the LargeList
  //! \return The overall status returned from sending the DDS messages required to perform this operation
  io::SendStatus popBack() {
    if (elements_.size() == 0) {
      return io::SendStatus::ERROR;
    }
    if (elements_.size() > 1) {
      auto newLast = elements_.rbegin()++;
      ListElement e = *newLast;
      e.nextElementID().reset();
      e.elementTimestamp(getTimestamp());
      if (elementSender_->send(e) == io::SendStatus::ERROR) {
        return io::SendStatus::ERROR;
      }
      *newLast = e;
    }

    if (elementSender_->dispose(elements_.back()) == io::SendStatus::ERROR) {
      UMAA_LOG_ERROR(util::getLogger(), "Unable to remove element from large list. List is likely no longer valid.");
      return io::SendStatus::ERROR;
    }
    auto removed = elements_.back();
    elements_.pop_back();

    updateMetadata(removed, true);

    return io::SendStatus::SUCCESS;
  }

  //! \brief Remove the first element from the LargeList
  //! \return The overall status returned from sending the DDS messages required to perform this operation
  io::SendStatus popFront() {
    if (elements_.size() == 0) {
      return io::SendStatus::ERROR;
    }

    if (elementSender_->dispose(elements_.front()) == io::SendStatus::ERROR) {
      return io::SendStatus::ERROR;
    }
    auto removed = elements_.front();
    elements_.pop_front();

    updateMetadata(removed, true);

    return io::SendStatus::SUCCESS;
  }

  //! \brief Remove an element from the LargeList at a given position
  //! \param pos The position in the list of the element to be removed
  //! \return The overall status returned from sending the DDS messages required to perform this operation
  io::SendStatus remove(uint64_t pos) {
    if (elements_.size() == 0 || pos > elements_.size()) {
      return io::SendStatus::ERROR;
    }
    if (pos == 0) {
      return popFront();
    }
    if (pos == elements_.size() - 1) {
      return popBack();
    }

    // Get the iterator to the element prior to the element in the position being removed
    typename std::list<ListElement>::iterator it = elements_.begin();
    std::advance(it, pos - 1);

    // Keep a copy of the iterator to the preceding and removed element while advancing the original
    auto prev = it;
    auto remove = std::next(it);
    auto next = std::next(remove);

    // Update the preceding element to point to the new next element and its timestamp then send the update
    prev->elementTimestamp(getTimestamp());
    prev->nextElementID(next->elementID());
    if (elementSender_->send(*prev) == io::SendStatus::ERROR) {
      return io::SendStatus::ERROR;
    }

    ListElement removed = *remove;
    if (elementSender_->dispose(removed) == io::SendStatus::ERROR) {
      UMAA_LOG_ERROR(util::getLogger(), "Unable to remove element from large list. List is likely no longer valid.");
      return io::SendStatus::ERROR;
    }
    elements_.erase(remove);

    updateMetadata(removed, true);

    return io::SendStatus::SUCCESS;
  }

  //! \brief Clears the LargeList by disposing all of its elements
  //! \return The overall status returned from sending the DDS messages required to perform this operation
  io::SendStatus clear() {
    io::SendStatus status = io::SendStatus::SUCCESS;
    while (elements_.size() > 0 && status == io::SendStatus::SUCCESS) {
      status = popFront();
    }
    return status;
  }

  //! \brief Get the metadata corresponding to the working state of the LargeList, potentially with pending operations
  //! \return The LargeList metadata object
  const UMAA::Common::LargeListMetadata getMetadata() const { return metadata_; }

  //! \brief Get the ID corresponding to the LargeList associated with this writer
  //! \return A NumericGuid representation of the listID
  const NumericGuid getListId() const { return listId_; }

  std::optional<Element> elementAt(uint64_t pos) const {
    if (pos >= elements_.size()) {
      return std::nullopt;
    }
    auto it = elements_.begin();
    std::advance(it, pos);
    return it->element();
  }

  //! \brief Returns a const reference to the first ListElement of the LargeList
  //! \return const reference to the first ListElement
  const ListElement& front() const {
    return elements_.front();
  }

  //! \brief Returns a const reference to the last ListElement of the LargeList
  //! \return const reference to the last ListElement
  const ListElement& back() const {
    return elements_.back();
  }

  //! \brief Returns a const iterator to the first ListElement of the LargeList
  //!        If the LargeList is empty, the returned iterator will be equal to \ref end()
  //! \return const iterator to the first ListElement
  typename std::list<ListElement>::const_iterator begin() const noexcept {
    return elements_.begin();
  }

  //! \brief Returns a const iterator to the first ListElement of the LargeList
  //!        If the LargeList is empty, the returned iterator will be equal to \ref cend()
  //! \return const iterator to the first ListElement
  typename std::list<ListElement>::const_iterator cbegin() const noexcept {
    return elements_.cbegin();
  }

  //! \brief Returns a const iterator to the element following the last ListElement of the LargeList
  //!        This element acts as a placeholder; attempting to access it results in undefined behavior
  //! \return const iterator to the element following the last ListElement
  typename std::list<ListElement>::const_iterator end() const noexcept {
    return elements_.end();
  }

  //! \brief Returns a const iterator to the element following the last ListElement of the LargeList
  //!        This element acts as a placeholder; attempting to access it results in undefined behavior
  //! \return const iterator to the element following the last ListElement
  typename std::list<ListElement>::const_iterator cend() const noexcept {
    return elements_.cend();
  }

  //! \brief Returns a const reverse iterator to the first ListElement of the reverse of the LargeList.
  //!        It corresponds to the last ListElement of the non-reversed list.
  //!        If the LargeList is empty, the returned iterator will be equal to \ref rend()
  //! \return const reverse iterator to the first ListElement
  typename std::list<ListElement>::const_reverse_iterator rbegin() const noexcept {
    return elements_.rbegin();
  }

  //! \brief Returns a const reverse iterator to the first ListElement of the reverse of the LargeList.
  //!        It corresponds to the last ListElement of the non-reversed list.
  //!        If the LargeList is empty, the returned iterator will be equal to \ref crend()
  //! \return const reverse iterator to the first ListElement
  typename std::list<ListElement>::const_reverse_iterator crbegin() const noexcept {
    return elements_.crbegin();
  }

  //! \brief Returns a const reverse iterator to the element following the last ListElement of the reverse of the
  //!        LargeList. It corresponds to the element preceding the first ListElement of the non-reversed list.
  //!        This element acts as a placeholder; attempting to access it results in undefined behavior
  //! \return const reverse iterator to the element following the last ListElement
  typename std::list<ListElement>::const_reverse_iterator rend() const noexcept {
    return elements_.rend();
  }

  //! \brief Returns a const reverse iterator to the element following the last ListElement of the reverse of the
  //!        LargeList. It corresponds to the element preceding the first ListElement of the non-reversed list.
  //!        This element acts as a placeholder; attempting to access it results in undefined behavior
  //! \return const reverse iterator to the element following the last ListElement
  typename std::list<ListElement>::const_reverse_iterator crend() const noexcept {
    return elements_.crend();
  }

  //! \brief Checks if the LargeList has no elements, i.e. whether begin() == end()
  //! \return true if the container is empty, false otherwise
  bool empty() const noexcept {
    return elements_.empty();
  }

  //! \brief Returns the number of unique ListElements (i.e. ListElements with the same ElementID) in the
  //!        LargeList, i.e. std::distance(begin(), end()).
  //! \return The number of ListElements in the LargeList
  std::size_t size() const noexcept {
    return elements_.size();
  }

  //! \brief Returns the maximum number of ListElements that the underlying container is able to hold due to
  //!        system or library implementation limitations, i.e. std::distance(begin(), end()) for the largest container.
  //! \return Maximum number of elements
  std::size_t max_size() const noexcept {
    return elements_.max_size();
  }

 private:
  std::list<ListElement> elements_;
  std::shared_ptr<io::SenderBase<ListElement>> elementSender_;
  UMAA::Common::LargeListMetadata metadata_;
  NumericGuid listId_;

  void updateMetadata(const ListElement& e, bool dispose = false) {
    metadata_.updateElementID(e.elementID());
    if (dispose) {
      metadata_.updateElementTimestamp().reset();
    } else {
      metadata_.updateElementTimestamp(e.elementTimestamp());
    }
    auto size = elements_.size();
    metadata_.size(size);
    if (size == 0) {
      metadata_.startingElementID(NIL_GUID.getGuid());
    } else {
      metadata_.startingElementID(elements_.front().elementID());
    }
  }

  ListElement newListElement(const Element& value) {
    auto newElementId = UuidFactory::getInstance().generateGuid().getGuid();
    ListElement e;
    e.element(value);
    e.listID(listId_.getGuid());
    e.elementID(newElementId);
    return e;
  }
};

}  // namespace arlcore::umaa

#endif  // INCLUDE_UMAA_LARGELISTWRITER_H_
