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

#ifndef INCLUDE_UMAA_SPECIALIZATIONS_UTILS_OBJECTIVESPECIALIZATIONUTILS_H_
#define INCLUDE_UMAA_SPECIALIZATIONS_UTILS_OBJECTIVESPECIALIZATIONUTILS_H_

#include <string>
#include <UMAA/MM/BaseType/ObjectiveType.hpp>
#include <UMAA/MM/BaseType/AreaRandomWalkObjectiveType.hpp>
#include <UMAA/MM/BaseType/CircleObjectiveType.hpp>
#include <UMAA/MM/BaseType/DriftObjectiveType.hpp>
#include <UMAA/MM/BaseType/ExpObjectiveType.hpp>
#include <UMAA/MM/BaseType/Figure8ObjectiveType.hpp>
#include <UMAA/MM/BaseType/FreeFloatObjectiveType.hpp>
#include <UMAA/MM/BaseType/HoverObjectiveType.hpp>
#include <UMAA/MM/BaseType/RacetrackObjectiveType.hpp>
#include <UMAA/MM/BaseType/RegularPolygonObjectiveType.hpp>
#include <UMAA/MM/BaseType/RouteObjectiveType.hpp>
#include <UMAA/MM/BaseType/ScreenRandomWalkObjectiveType.hpp>
#include <UMAA/MM/BaseType/StationkeepObjectiveType.hpp>
#include <UMAA/MM/BaseType/VectorObjectiveType.hpp>

using UMAA::MM::BaseType::ObjectiveType;
using UMAA::MM::BaseType::AreaRandomWalkObjectiveTypeTopic;
using UMAA::MM::BaseType::AreaRandomWalkObjectiveType;
using UMAA::MM::BaseType::CircleObjectiveTypeTopic;
using UMAA::MM::BaseType::CircleObjectiveType;
using UMAA::MM::BaseType::DriftObjectiveTypeTopic;
using UMAA::MM::BaseType::DriftObjectiveType;
using UMAA::MM::BaseType::ExpObjectiveTypeTopic;
using UMAA::MM::BaseType::ExpObjectiveType;
using UMAA::MM::BaseType::Figure8ObjectiveTypeTopic;
using UMAA::MM::BaseType::Figure8ObjectiveType;
using UMAA::MM::BaseType::FreeFloatObjectiveTypeTopic;
using UMAA::MM::BaseType::FreeFloatObjectiveType;
using UMAA::MM::BaseType::HoverObjectiveTypeTopic;
using UMAA::MM::BaseType::HoverObjectiveType;
using UMAA::MM::BaseType::RacetrackObjectiveTypeTopic;
using UMAA::MM::BaseType::RacetrackObjectiveType;
using UMAA::MM::BaseType::RegularPolygonObjectiveTypeTopic;
using UMAA::MM::BaseType::RegularPolygonObjectiveType;
using UMAA::MM::BaseType::RouteObjectiveTypeTopic;
using UMAA::MM::BaseType::RouteObjectiveType;
using UMAA::MM::BaseType::ScreenRandomWalkObjectiveTypeTopic;
using UMAA::MM::BaseType::ScreenRandomWalkObjectiveType;
using UMAA::MM::BaseType::StationkeepObjectiveTypeTopic;
using UMAA::MM::BaseType::StationkeepObjectiveType;
using UMAA::MM::BaseType::VectorObjectiveTypeTopic;
using UMAA::MM::BaseType::VectorObjectiveType;
using UMAA::MM::BaseType::RouteObjectiveTypeTopic;
using UMAA::MM::BaseType::WaypointType;
using UMAA::MM::BaseType::RouteObjectiveTypeWaypointsListElement;
using UMAA::MM::BaseType::RouteObjectiveTypeWaypointsListElementTopic;

namespace arlcore::umaa {

enum class ObjectiveSpecializationEnum {
  AREA_RAND_WALK,
  CIRCLE,
  DRIFT,
  EXP,
  FIGURE8,
  FREE_FLOAT,
  HOVER,
  RACETRACK,
  REGULAR_POLYGON,
  ROUTE,
  SCREEN_RAND_WALK,
  STATIONKEEP,
  VECTOR,
  ERROR
};

inline std::string toString(const ObjectiveSpecializationEnum& input) {
  switch (input) {
    case ObjectiveSpecializationEnum::AREA_RAND_WALK:   return AreaRandomWalkObjectiveTypeTopic;
    case ObjectiveSpecializationEnum::CIRCLE:           return CircleObjectiveTypeTopic;
    case ObjectiveSpecializationEnum::DRIFT:            return DriftObjectiveTypeTopic;
    case ObjectiveSpecializationEnum::EXP:              return ExpObjectiveTypeTopic;
    case ObjectiveSpecializationEnum::FIGURE8:          return Figure8ObjectiveTypeTopic;
    case ObjectiveSpecializationEnum::FREE_FLOAT:       return FreeFloatObjectiveTypeTopic;
    case ObjectiveSpecializationEnum::HOVER:            return HoverObjectiveTypeTopic;
    case ObjectiveSpecializationEnum::RACETRACK:        return RacetrackObjectiveTypeTopic;
    case ObjectiveSpecializationEnum::REGULAR_POLYGON:  return RegularPolygonObjectiveTypeTopic;
    case ObjectiveSpecializationEnum::ROUTE:            return RouteObjectiveTypeTopic;
    case ObjectiveSpecializationEnum::SCREEN_RAND_WALK: return ScreenRandomWalkObjectiveTypeTopic;
    case ObjectiveSpecializationEnum::STATIONKEEP:      return StationkeepObjectiveTypeTopic;
    case ObjectiveSpecializationEnum::VECTOR:           return VectorObjectiveTypeTopic;
    default: return "";
  }
}

inline ObjectiveSpecializationEnum getObjSpecFromTopic(std::string_view topic) {
  if (topic == AreaRandomWalkObjectiveTypeTopic)        return ObjectiveSpecializationEnum::AREA_RAND_WALK;
  else if (topic == CircleObjectiveTypeTopic)           return ObjectiveSpecializationEnum::CIRCLE;
  else if (topic == DriftObjectiveTypeTopic)            return ObjectiveSpecializationEnum::DRIFT;
  else if (topic == ExpObjectiveTypeTopic)              return ObjectiveSpecializationEnum::EXP;
  else if (topic == Figure8ObjectiveTypeTopic)          return ObjectiveSpecializationEnum::FIGURE8;
  else if (topic == FreeFloatObjectiveTypeTopic)        return ObjectiveSpecializationEnum::FREE_FLOAT;
  else if (topic == HoverObjectiveTypeTopic)            return ObjectiveSpecializationEnum::HOVER;
  else if (topic == RacetrackObjectiveTypeTopic)        return ObjectiveSpecializationEnum::RACETRACK;
  else if (topic == RegularPolygonObjectiveTypeTopic)   return ObjectiveSpecializationEnum::REGULAR_POLYGON;
  else if (topic == RouteObjectiveTypeTopic)            return ObjectiveSpecializationEnum::ROUTE;
  else if (topic == ScreenRandomWalkObjectiveTypeTopic) return ObjectiveSpecializationEnum::SCREEN_RAND_WALK;
  else if (topic == StationkeepObjectiveTypeTopic)      return ObjectiveSpecializationEnum::STATIONKEEP;
  else if (topic == VectorObjectiveTypeTopic)           return ObjectiveSpecializationEnum::VECTOR;
  else  return ObjectiveSpecializationEnum::ERROR;
}

}  // namespace arlcore::umaa

#endif  // INCLUDE_UMAA_SPECIALIZATIONS_UTILS_OBJECTIVESPECIALIZATIONUTILS_H_
