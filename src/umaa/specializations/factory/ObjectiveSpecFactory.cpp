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

#include "ObjectiveSpecFactory.h"

namespace arlcore::umaa {

  shared_ptr<SendableSpecializationBase> ObjectiveSpecFactory::createSpecialization(const ObjectiveType& obj) {
    std::string specTopic = obj.specializationTopic();
    switch (getObjSpecFromTopic(specTopic)) {
      case ObjectiveSpecializationEnum::AREA_RAND_WALK:
        return makeSpecialization<AreaRandomWalkObjectiveType>(obj);
      case ObjectiveSpecializationEnum::CIRCLE:
        return makeSpecialization<CircleObjectiveType>(obj);
      case ObjectiveSpecializationEnum::DRIFT:
        return makeSpecialization<DriftObjectiveType>(obj);
      case ObjectiveSpecializationEnum::EXP:
        return makeSpecialization<ExpObjectiveType>(obj);
      case ObjectiveSpecializationEnum::FIGURE8:
        return makeSpecialization<Figure8ObjectiveType>(obj);
      case ObjectiveSpecializationEnum::FREE_FLOAT:
        return makeSpecialization<FreeFloatObjectiveType>(obj);
      case ObjectiveSpecializationEnum::HOVER:
        return makeSpecialization<HoverObjectiveType>(obj);
      case ObjectiveSpecializationEnum::RACETRACK:
        return makeSpecialization<RacetrackObjectiveType>(obj);
      case ObjectiveSpecializationEnum::REGULAR_POLYGON:
        return makeSpecialization<RegularPolygonObjectiveType>(obj);
      case ObjectiveSpecializationEnum::ROUTE:
        return makeRoute(obj);
      case ObjectiveSpecializationEnum::SCREEN_RAND_WALK:
        return makeSpecialization<ScreenRandomWalkObjectiveType>(obj);
      case ObjectiveSpecializationEnum::STATIONKEEP:
        return makeSpecialization<StationkeepObjectiveType>(obj);
      case ObjectiveSpecializationEnum::VECTOR:
        return makeSpecialization<VectorObjectiveType>(obj);
      default:
        UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "ObjectiveType with specialization topic "
          + specTopic + " is not supported.")
        return nullptr;
    }
  }

  shared_ptr<SendableSpecializationBase> ObjectiveSpecFactory::makeRoute(const ObjectiveType& obj) {
    auto reader = readerRegistry_->getReader<RouteObjectiveType>(obj.specializationTopic());
    auto writer = writerRegistry_->getWriter<RouteObjectiveType>(obj.specializationTopic());
    auto wptListElemWriter = writerRegistry_->getWriter<RouteObjectiveTypeWaypointsListElement>(
                                                        RouteObjectiveTypeWaypointsListElementTopic);
    // Check that necessary readers and writers exists.
    if (!reader || !writer || !wptListElemWriter || !routeWptReader_) {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Reader or Writer not found for " << obj.specializationTopic())
      return nullptr;
    }

    auto readSpec = readSpecialization<RouteObjectiveType>(obj, reader);
    if (!readSpec) {
      return nullptr;
    }

    auto wptList = routeWptReader_->getListFromMetadata(readSpec->waypointsListMetadata());
    if (wptList.status != LargeListStatus::VALID_LIST || wptList.list.expired()) {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "Unable to create valid large list from Route Objective data")
      return nullptr;
    }

    return make_shared<RouteSendableSpec>(readSpec.value(), *wptList.list.lock(), writer, wptListElemWriter);
  }

}  // namespace arlcore::umaa
