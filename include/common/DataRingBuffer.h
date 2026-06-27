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

#ifndef INCLUDE_COMMON_DATARINGBUFFER_H_
#define INCLUDE_COMMON_DATARINGBUFFER_H_

#include <array>
#include <cstdint>
#include <iostream>
#include <memory>
#include <mutex>
#include <vector>

#include "Logger.h"

namespace arlcore {

//! \brief FIFO Ring Buffer with methods similar to a std::queue
template<class T>
class DataRingBuffer {
 public:
  DataRingBuffer() = delete;

  explicit DataRingBuffer(size_t size) :
      maxSize_(size), buffer_(maxSize_) {
    // Variables initialized above
  }

  //! \brief Push an item onto the queue.
  //! \param The item reference to put onto the queue.
  void push(const T& item) {
    std::lock_guard<std::mutex> lock(mutex_);

    buffer_[head_] = item;

    if (full_) {
      UMAA_LOG_TRACE(util::SYSTEM_LOGGER, "Buffer Full, overwriting tail... ")
      tail_ = ((tail_ + 1) % maxSize_);
    }

    head_ = (head_ + 1) % maxSize_;
    full_ = head_ == tail_;
  }

  //! \brief Get the next item in the queue.
  //!  This will pop the item off the queue.
  //! \return A copy of the item next in the queue buffer.
  T get() {
    uint32_t retIndex = 0;

    std::lock_guard<std::mutex> lock(mutex_);
    if (!isEmptyInternal()) {
      retIndex = tail_;
      full_ = false;
      tail_ = (tail_ + 1) % maxSize_;
    }

    return buffer_[retIndex];
  }

  //! \brief Get a copy of the item from the front of the queue.
  //! \return A copy of the item held in the queue buffer.
  T front() const {
    return front(0);
  }

  //! \brief Get a copy of the item from the front of the queue.
  //! \param The index to move from the front backward.
  //! \return A copy of the item held in the queue buffer.
  T front(uint32_t frontIndex) const {
    std::lock_guard<std::mutex> lock(mutex_);
    uint32_t bufferIndex = tail_ + frontIndex % maxSize_;
    return buffer_.at(bufferIndex);
  }

  //! \brief Get a copy of the item from the back of the queue.
  //! \return A copy of the item held in the queue buffer.
  T back() const {
    return back(0);
  }

  //! \brief Get a copy of the item from the back of the queue.
  //! \param The index to move from the back forward.
  //! \return A copy of the item held in the queue buffer.
  T back(uint32_t backIndex) const {
    uint32_t bufferIndex = 0;

    std::lock_guard<std::mutex> lock(mutex_);

    // Make sure the backIndex is not greater than max size
    backIndex = backIndex % maxSize_;

    uint32_t currHead = 0;
    if (head_ == 0) {
      currHead = maxSize_ - 1;
    } else {
      currHead = head_ - 1;
    }

    if (backIndex > currHead) {
      bufferIndex = maxSize_ - (backIndex - currHead);
    } else {
      bufferIndex = currHead - backIndex;
    }

    return buffer_.at(bufferIndex);
  }

  //! \brief Pop next item off the queue.
  void pop() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!isEmptyInternal()) {
      full_ = false;
      tail_ = (tail_ + 1) % maxSize_;
    }
  }

  //! \brief Clear all elements off the queue.
  void clear() {
    std::lock_guard<std::mutex> lock(mutex_);

    head_ = 0;
    tail_ = 0;
    full_ = false;
  }

  bool isEmpty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return isEmptyInternal();
  }

  bool isFull() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return full_;
  }

  uint32_t getMaxCapacity() const {
    return maxSize_;
  }

  uint32_t size() const {
    std::lock_guard<std::mutex> lock(mutex_);

    uint32_t size = maxSize_;

    if (!full_) {
      if (head_ >= tail_) {
        size = head_ - tail_;
      } else {
        size = maxSize_ + head_ - tail_;
      }
    }

    return size;
  }

 private:
  bool isEmptyInternal() const {
    return (!full_ && (head_ == tail_));
  }

  mutable std::mutex mutex_;
  uint32_t maxSize_;
  std::vector<T> buffer_;
  uint32_t head_ = 0;
  uint32_t tail_ = 0;
  bool full_ = false;
};

}  // namespace arlcore

#endif  // INCLUDE_COMMON_DATARINGBUFFER_H_
