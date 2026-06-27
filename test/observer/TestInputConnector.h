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
#ifndef TEST_OBSERVER_TESTINPUTCONNECTOR_H_
#define TEST_OBSERVER_TESTINPUTCONNECTOR_H_

#include <memory>

#include "InputConnector.h"

namespace arlcore {

template<class T>
class TestInputConnector: public arlcore::InputConnector<T> {
 public:
  TestInputConnector() :
    arlcore::InputConnector<T>(std::make_shared<arlcore::Subject<T>>()){
  }

  virtual ~TestInputConnector() = default;

};

}  // namespace arlcore

#endif  // TEST_OBSERVER_TESTINPUTCONNECTOR_H_
