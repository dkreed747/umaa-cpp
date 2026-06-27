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

#ifndef INCLUDE_OBSERVER_SUBJECT_H_
#define INCLUDE_OBSERVER_SUBJECT_H_

#include <algorithm>
#include <iterator>
#include <memory>
#include <set>
#include <optional>

#include "Observer.h"

namespace arlcore {

template<class T>
class Subject {
 public:
  //! \brief Constructor
  //! \param resendLatest Whether to resend the latest notification to new observers (default true)
  explicit Subject(bool resendLatest = true) : resendLatest_(resendLatest) {}
  virtual ~Subject() = default;

  //! \brief Add an Observer to the subject which will notify them of updates.
  //! \param The Observer to be added to the notification list
  void registerObserver(std::weak_ptr<Observer<T>> obs) {
    if (std::shared_ptr<Observer<T>> ptr = obs.lock(); resendLatest_ && latest_.has_value()) {
      ptr->update(latest_.value());
    }
    observers_.insert(obs);
  }

  //! \brief Remove an Observer from the subject to no longer notify them of updates.
  //! \param The Observer to be removed from the notification list
  void unregisterObserver(std::weak_ptr<Observer<T>> obs) {
    observers_.erase(obs);
  }

  //! \brief Notification trigger of observers.
  //! This will iterate through the list of observers and notify them.
  //! \param inData The input data to transmit to observers.
  void notify(const T& inData) {
    latest_ = inData;
    // Notify valid observers and add invalid ones to a set to remove
    std::set<std::weak_ptr<Observer<T>>, std::owner_less<std::weak_ptr<Observer<T>>>> invalid_observers;
    for (std::weak_ptr<Observer<T>> obs : observers_) {
      if (std::shared_ptr<Observer<T>> ptr = obs.lock()) {
        ptr->update(inData);
      } else {
        invalid_observers.insert(obs);
      }
    }
    // Set to store valid observers
    std::set<std::weak_ptr<Observer<T>>, std::owner_less<std::weak_ptr<Observer<T>>>> valid_observers;

    // Populate valid_observers with the elements that are in observers_ but not in invalid_observers
    std::set_difference(observers_.begin(), observers_.end(), invalid_observers.begin(),
      invalid_observers.end(), std::inserter(valid_observers, valid_observers.end()),
      std::owner_less<std::weak_ptr<Observer<T>>>());

    // Set observers to valid_observers
    observers_ = valid_observers;
  }

  //! \brief Get the Number of observers registered against this subject.
  //! \return the number of observers registered against this subject.
  uint32_t getObserverCount() const {
    return observers_.size();
  }

  std::optional<T> getLatestNotification() const {
    return latest_;
  }

 private:
  std::set<std::weak_ptr<Observer<T>>, std::owner_less<std::weak_ptr<Observer<T>>>> observers_;
  bool resendLatest_;
  std::optional<T> latest_;
};

}  // namespace arlcore

#endif  // INCLUDE_OBSERVER_SUBJECT_H_
