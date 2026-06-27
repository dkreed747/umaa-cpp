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

#ifndef TEST_DOMAIN_TESTOBSERVER_H_
#define TEST_DOMAIN_TESTOBSERVER_H_

#include <queue>

#include "Observer.h"

namespace arlcoretest {

template<class T>
class TestObserver: public arlcore::Observer<T> {
 public:
  TestObserver() = default;
  virtual ~TestObserver() = default;

  void update(const T& data) override {
    gpQueue_.push(data);
  }

  //! \brief Get the data queue size.
  //! \return the size of the GlobalPoseData queue.
  uint32_t getDataCount() {
    return gpQueue_.size();
  }

  T getNextData() {
    T retData;

    if (!gpQueue_.empty()) {
      retData = gpQueue_.front();

      // pop off the front entry
      gpQueue_.pop();
    }

    return retData;
  }

  void clear() {
    std::queue<T>().swap(gpQueue_);
  }

 private:
  std::queue<T> gpQueue_;
};

}  // namespace arltest

#endif  // TEST_DOMAIN_TESTOBSERVER_H_
