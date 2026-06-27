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

#ifndef INCLUDE_UMAA_SPECIALIZATIONS_FACTORY_OBJECTIVESPECFACTORY_H_
#define INCLUDE_UMAA_SPECIALIZATIONS_FACTORY_OBJECTIVESPECFACTORY_H_

#include <memory>

#include <UMAA/Common/Measurement/Measurements.hpp>

#include "IOReaderRegistry.h"
#include "IOWriterRegistry.h"
#include "LargeListReader.h"
#include "LargeListWriter.h"
#include "RouteSendableSpec.h"
#include "SpecializationFactoryBase.h"

using arlcore::umaa::LargeListReader;
using arlcore::umaa::SendableSpecializationBase;
using std::make_shared;
using std::shared_ptr;
using UMAA::Common::Measurement::DateTime;
using UMAA::Common::Measurement::NumericGUID;

namespace arlcore::umaa {

//! \brief Factory for creating unique pointers of sendable specializations.
class ObjectiveSpecFactory : public SpecializationFactoryBase<ObjectiveType> {
 public:
  ObjectiveSpecFactory() = delete;

  //! \brief Constructor for the factory. This factory can be constructed with as many readers/writers as needed.
  //! For example, if you want to use this factory class as a util to read and create only HoverObjectiveTypes,
  //! you only need HoverObjectiveType readers and writers.
  //! \param readerReg An IOReaderRegistry with necessary readers.
  //! \param writerReg An IOWriterRegistry with necessary writers.
  ObjectiveSpecFactory(shared_ptr<IOReaderRegistry> readerReg, shared_ptr<IOWriterRegistry> writerReg)
      : SpecializationFactoryBase(readerReg, writerReg) {}

  //! \brief Sets the waypoint large list reader - this parameter is optional.
  void setWptLargeListReader(shared_ptr<LargeListReader<WaypointType, RouteObjectiveTypeWaypointsListElement>> reader) {
    routeWptReader_ = reader;
  }

  //! \brief Given an ObjectiveType, reads the specialization off the DDS bus and creates a sendable specialization
  //! Object.
  //! \param obj An ObjectiveType
  //! \return A sendable specialization. If the Factory does not have the required readers/writers to perform this
  //! operation, this function will return a null ptr. Callers of this function are responsible for validating the
  //! return.
  shared_ptr<SendableSpecializationBase> createSpecialization(const ObjectiveType& obj) override;

 private:
  //! \brief Utility function for reading off the DDS bus and creating a sendable Route specialization.
  //! \param obj The ObjectiveType
  //! \return A sendable specialization or a nullptr if unable to create one.
  shared_ptr<SendableSpecializationBase> makeRoute(const ObjectiveType& obj);

  //! Large List reader for RouteObjectiveTypeWaypointsListElement - not required if not reading RouteObjectiveTypes.
  shared_ptr<LargeListReader<WaypointType, RouteObjectiveTypeWaypointsListElement>> routeWptReader_;
};
}  // namespace arlcore::umaa
#endif  // INCLUDE_UMAA_SPECIALIZATIONS_FACTORY_OBJECTIVESPECFACTORY_H_
