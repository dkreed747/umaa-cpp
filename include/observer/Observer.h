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

#ifndef INCLUDE_OBSERVER_OBSERVER_H_
#define INCLUDE_OBSERVER_OBSERVER_H_

#include "GlobalPoseData.h"
#include "InternalTypes.h"

namespace arlcore {

template<class T>
class Observer {
 public:
  //! \brief Default Constructor
  Observer() = default;

  //! \brief Default Destructor
  virtual ~Observer() = default;

  //! \brief Observer function used to receive updates from a subject
  //! \param Data to be updated
  virtual void update(const T& gpData) = 0;
};

}  // namespace arlcore

#endif  // INCLUDE_OBSERVER_OBSERVER_H_
