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

#include "GlobalWaypointControlServiceConsumer.h"

namespace arlcore::umaa {

GlobalWaypointControlServiceConsumer::GlobalWaypointControlServiceConsumer(
  const CommandHeader& cmdHdr,
  std::shared_ptr<GlobalWaypointControlServiceConsumerIo> io) :
  CommandConsumerBase(cmdHdr, io),
  LargeListWriter(io->listElementSender) {}

GlobalWaypointControlServiceConsumer::GlobalWaypointControlServiceConsumer(
  const CommandHeader& cmdHdr,
  std::shared_ptr<GlobalWaypointControlServiceConsumerIo> io,
  const std::vector<GlobalWaypointType>& waypoints) :
  CommandConsumerBase(cmdHdr, io),
  LargeListWriter(io->listElementSender, waypoints) {}

GlobalWaypointControlServiceConsumer::~GlobalWaypointControlServiceConsumer() {
  if (isSessionOpen_) {
    UMAA_LOG_DEBUG(util::SYSTEM_LOGGER, "Closing Waypoint Command session on shutdown")
    closeCommandSession();
  }
}

SendStatus GlobalWaypointControlServiceConsumer::send(GlobalWaypointCommandType *cmd) {
  if (cmd == nullptr) {
    UMAA_LOG_ERROR(util::SYSTEM_LOGGER, "cmd pointer is null!")
    return SendStatus::ERROR;
  }

  // Set associated large list with the metadata
  cmd->waypointsListMetadata(getMetadata());

  return CommandConsumerBase::send(cmd);
}

SendStatus GlobalWaypointControlServiceConsumer::sessionTearDown() {
  return clear();
}
}  // namespace arlcore::umaa
