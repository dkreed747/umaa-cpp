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

#ifndef INCLUDE_UMAA_SPECIALIZATIONS_DATA_ROUTESENDABLESPEC_H_
#define INCLUDE_UMAA_SPECIALIZATIONS_DATA_ROUTESENDABLESPEC_H_

#include <list>
#include <memory>
#include <string>

#include "DefaultSendableSpec.h"
#include "LargeListWriter.h"
#include "ObjectiveSpecializationUtils.h"

using arlcore::umaa::LargeListWriter;

namespace arlcore::umaa {

//! \brief A class to store and send Route Objective and Waypoint data.
//! \param spec The Route Objective Type
//! \param wpts A list of waypoints
//! \param routeSender A sender for RouteObjectiveType
//! \param wptSender A sender for RouteObjectiveTypeWaypointsListElement
class RouteSendableSpec final : public DefaultSendableSpec<RouteObjectiveType> {
 public:
  RouteSendableSpec(const RouteObjectiveType& spec,
                    const std::list<WaypointType>& wpts,
                    const shared_ptr<SenderBase<RouteObjectiveType>>& routeSender,
                    const shared_ptr<SenderBase<RouteObjectiveTypeWaypointsListElement>>& wptSender)
      : DefaultSendableSpec(spec, routeSender, RouteObjectiveTypeTopic),
        waypoints_(wpts),
        wptSender_(wptSender),
        wptListWriter_(
            std::make_unique<LargeListWriter<WaypointType, RouteObjectiveTypeWaypointsListElement>>(wptSender)) {}

  //! \brief Deep copy constructor for RouteSendableSpec.
  //!  When copying a RouteSendableSpec class, a new LargeListWriter must be made as to not overwrite the
  //! List elements sent by the previous RouteSendableSpec.
  RouteSendableSpec(const RouteSendableSpec& other)
      : DefaultSendableSpec(other),
        waypoints_(other.waypoints_),
        wptSender_(other.wptSender_),
        wptListWriter_(
            std::make_unique<LargeListWriter<WaypointType, RouteObjectiveTypeWaypointsListElement>>(wptSender_)) {
    updateMetadata();
  }

  std::shared_ptr<SendableSpecializationBase> with(const NumericGUID& specID,
                                                   const DateTime& timestamp) const override {
    auto specCopy = specData_;
    specCopy.specializationReferenceID(specID);
    specCopy.specializationReferenceTimestamp(timestamp);
    return std::make_shared<RouteSendableSpec>(specCopy, this->waypoints_, this->specSender_, this->wptSender_);
  }

  std::list<WaypointType> getWaypoints() const { return waypoints_; }

  SendStatus sendDataSpecific() override {
    for (const auto& wpt : waypoints_) {
      if (wptListWriter_->append(wpt) != SendStatus::SUCCESS) {
        wptListWriter_->clear();
        return SendStatus::ERROR;
      }
    }
    updateMetadata();
    return SendStatus::SUCCESS;
  }

  SendStatus disposeDataSpecific() override {
    if (wptListWriter_->clear() != SendStatus::SUCCESS) {
      return SendStatus::ERROR;
    }
    updateMetadata();
    return SendStatus::SUCCESS;
  }

 private:
  //! Specialization Data is stored in the base class
  //! \brief Updates the stored RouteObjectiveType with the metadata from the Waypoint List Writer
  //! The metadata will match what is on the bus.
  //! For example, If this class has 4 waypoints, but none have been sent to the bus
  //! via wpt list writer, the metadata will have a size of 0.
  void updateMetadata() { specData_.waypointsListMetadata(wptListWriter_->getMetadata()); }

  //! List of RouteObjectiveType Waypoints
  std::list<WaypointType> waypoints_;
  //! Shared sender for RouteObjectiveTypeWaypointsListElements. Stored for copying.
  shared_ptr<SenderBase<RouteObjectiveTypeWaypointsListElement>> wptSender_;
  //! Unique LargeListWriter. One Large List Writer is allowed per class, but they can all share the Waypoint Sender.
  std::unique_ptr<LargeListWriter<WaypointType, RouteObjectiveTypeWaypointsListElement>> wptListWriter_;
};
}  // namespace arlcore::umaa


#endif  // INCLUDE_UMAA_SPECIALIZATIONS_DATA_ROUTESENDABLESPEC_H_
