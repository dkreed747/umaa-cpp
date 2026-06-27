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

#ifndef INCLUDE_OBSERVER_INPUTCONNECTOR_H_
#define INCLUDE_OBSERVER_INPUTCONNECTOR_H_

#include <memory>

#include "Subject.h"

namespace arlcore {
//! \brief Class for handling incoming data from hardware.
//!  This function signals for the subject to notify the Observers with the
//!  new data received.
//!
template<class T>
class InputConnector {
 public:
  //! \brief Delete the default constructor
  InputConnector() = delete;

  //! \brief Template class for handling incoming data from hardware
  //! \param Subject instance for notifying update
  explicit InputConnector(std::shared_ptr<Subject<T>> subject) :
      subject_(subject) {
  }

  //! \brief Default Destructor
  virtual ~InputConnector() = default;

  //! \brief Part of Observer Pattern that notifies observers of events
  //! \param New incoming data
  virtual void trigger(const T& inData) {
    setData(inData);
    subject_->notify(inData);
  }

  //! \brief Set new data
  //! \param New incoming data
  void setData(const T& inData) {
    lastData_ = inData;
  }

  //! \brief Return last received data
  //! \return Last received data
  const T& getLastData() {
    return lastData_;
  }

  //! \brief Get Subject
  //! \return Subject
  std::shared_ptr<Subject<T>> getSubject() {
    return subject_;
  }

 private:
  std::shared_ptr<Subject<T>> subject_;
  T lastData_;
};

}  // namespace arlcore

#endif  // INCLUDE_OBSERVER_INPUTCONNECTOR_H_
