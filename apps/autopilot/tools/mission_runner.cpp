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

//! \brief End-to-end waypoint mission driver for the autopilot. Acts as the UMAA consumer
//! side: publishes a GlobalWaypointCommandType (destination = the autopilot's waypoint
//! provider) plus its large-list route, then records the vehicle's Global Pose track and the
//! command status until the mission completes. Outputs:
//!   <out>/track.csv      elapsed_s, lat_deg, lon_deg, yaw_rad, speed_mps
//!   <out>/waypoints.csv  index, lat_deg, lon_deg, capture_radius_m
//!   <out>/status.log     command status transitions
//! Usage: mission_runner [autopilot.yaml] [output-dir]

#include <chrono>
#include <cmath>
#include <memory>
#include <filesystem>  // NOLINT(build/c++17)
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <GeographicLib/LocalCartesian.hpp>

#include <UMAA/MO/GlobalWaypointControl/GlobalWaypointCommandType.hpp>
#include <UMAA/MO/GlobalWaypointControl/GlobalWaypointCommandStatusType.hpp>
#include <UMAA/MO/GlobalWaypointControl/GlobalWaypointExecutionStatusReportType.hpp>
#include <UMAA/SA/GlobalPoseStatus/GlobalPoseReportType.hpp>
#include <UMAA/SA/SpeedStatus/SpeedReportType.hpp>

#include "AutopilotConfig.h"
#include "CycloneQosProviderWrapper.h"
#include "CycloneReader.h"
#include "CycloneSender.h"
#include "CycloneUtilities.h"
#include "LargeListWriter.h"
#include "Logger.h"
#include "NumericGuid.h"
#include "UmaaUtils.h"
#include "UuidFactory.h"
#include "YamlConfigLoader.h"

namespace {

using arlcore::io::CycloneReader;
using arlcore::io::CycloneSender;
using arlcore::io::ReadStatus;
using UMAA::MO::GlobalWaypointControl::GlobalWaypointCommandType;
using UMAA::MO::GlobalWaypointControl::GlobalWaypointCommandStatusType;
using UMAA::MO::GlobalWaypointControl::GlobalWaypointCommandTypeWaypointsListElement;
using UMAA::MO::GlobalWaypointControl::GlobalWaypointType;
using UMAA::SA::GlobalPoseStatus::GlobalPoseReportType;
using UMAA::SA::SpeedStatus::SpeedReportType;
using CommandStatusEnumType =
    UMAA::Common::MaritimeEnumeration::CommandStatusEnumModule::CommandStatusEnumType;

//! \brief One mission waypoint expressed in the local tangent plane at the sim start.
struct LocalWaypoint {
  double eastM;
  double northM;
  double speedMps;
  double captureRadiusM;
};

GlobalWaypointType makeWaypoint(const GeographicLib::LocalCartesian& frame, const LocalWaypoint& lw) {
  double lat = 0.0;
  double lon = 0.0;
  double h = 0.0;
  frame.Reverse(lw.eastM, lw.northM, 0.0, lat, lon, h);

  GlobalWaypointType wp;
  wp.position().value().geodeticLatitude(lat);
  wp.position().value().geodeticLongitude(lon);
  UMAA::Common::Position::GeoPosition2DTolerance tol;
  tol.limit(lw.captureRadiusM);
  wp.position().tolerance() = tol;
  wp.speed().VariableSpeedVariantTypeSubtypes().RequiredSpeedVariantVariant(
      UMAA::Common::Speed::RequiredSpeedVariantType());
  wp.speed().VariableSpeedVariantTypeSubtypes().RequiredSpeedVariantVariant().speed()
      .SpeedRequirementVariantTypeSubtypes().GroundSpeedRequirementVariantVariant(
          UMAA::Common::Speed::GroundSpeedRequirementVariantType());
  wp.speed().VariableSpeedVariantTypeSubtypes().RequiredSpeedVariantVariant().speed()
      .SpeedRequirementVariantTypeSubtypes().GroundSpeedRequirementVariantVariant().speed()
      .speed(lw.speedMps);
  wp.waypointID() = arlcore::UuidFactory::getInstance().generateGuid().getGuid();
  return wp;
}

std::string statusName(CommandStatusEnumType s) {
  switch (s) {
    case CommandStatusEnumType::ISSUED: return "ISSUED";
    case CommandStatusEnumType::COMMANDED: return "COMMANDED";
    case CommandStatusEnumType::EXECUTING: return "EXECUTING";
    case CommandStatusEnumType::COMPLETED: return "COMPLETED";
    case CommandStatusEnumType::CANCELED: return "CANCELED";
    case CommandStatusEnumType::FAILED: return "FAILED";
    default: return "UNKNOWN";
  }
}

}  // namespace

int main(int argc, char** argv) {
  const std::string configPath = (argc > 1) ? argv[1] : "autopilot.yaml";
  const std::string outDir = (argc > 2) ? argv[2] : "mission-out";

  arlcore::autopilot::AutopilotConfig config;
  if (!arlcore::autopilot::YamlConfigLoader::load(configPath, &config)) {
    std::cerr << "Failed to load config " << configPath << std::endl;
    return 1;
  }
  std::filesystem::create_directories(outDir);

  auto participant = arlcore::io::getDomainParticipant(config.dds.domainId);
  arlcore::io::CycloneQosProviderWrapper qosProvider(config.dds.qosFile, config.dds.domainQosProfile);
  const auto rqos = qosProvider.datareader_qos();
  const auto wqos = qosProvider.datawriter_qos();

  // The mission: a closed loop with turns in both directions, relative to the sim start.
  GeographicLib::LocalCartesian frame(config.simVehicle.initialLatitudeDeg,
                                      config.simVehicle.initialLongitudeDeg, 0.0);
  const double v = 3.0;
  const std::vector<LocalWaypoint> localRoute = {
      {0.0, 350.0, v, 12.0},
      {250.0, 600.0, v, 12.0},
      {500.0, 350.0, v, 12.0},
      {250.0, 100.0, v, 12.0},
      {-50.0, 350.0, v, 12.0},
  };
  std::vector<GlobalWaypointType> waypoints;
  for (const LocalWaypoint& lw : localRoute) {
    waypoints.push_back(makeWaypoint(frame, lw));
  }

  {
    std::ofstream wpCsv(outDir + "/waypoints.csv");
    wpCsv.precision(10);
    wpCsv << "index,lat_deg,lon_deg,capture_radius_m\n";
    for (std::size_t i = 0; i < waypoints.size(); i++) {
      wpCsv << i << "," << waypoints[i].position().value().geodeticLatitude() << ","
            << waypoints[i].position().value().geodeticLongitude() << ","
            << localRoute[i].captureRadiusM << "\n";
    }
  }

  // IO: command + list element writers, status + nav readers.
  auto cmdSender = std::make_shared<CycloneSender<GlobalWaypointCommandType>>(
      participant, UMAA::MO::GlobalWaypointControl::GlobalWaypointCommandTypeTopic, wqos);
  auto elementSender = std::make_shared<CycloneSender<GlobalWaypointCommandTypeWaypointsListElement>>(
      participant, UMAA::MO::GlobalWaypointControl::GlobalWaypointCommandTypeWaypointsListElementTopic, wqos);
  auto statusReader = std::make_shared<CycloneReader<GlobalWaypointCommandStatusType>>(
      participant, UMAA::MO::GlobalWaypointControl::GlobalWaypointCommandStatusTypeTopic, rqos);
  auto poseReader = std::make_shared<CycloneReader<GlobalPoseReportType>>(
      participant, UMAA::SA::GlobalPoseStatus::GlobalPoseReportTypeTopic, rqos);
  auto speedReader = std::make_shared<CycloneReader<SpeedReportType>>(
      participant, UMAA::SA::SpeedStatus::SpeedReportTypeTopic, rqos);

  // Publish the route as a UMAA large list, then the command referencing it.
  arlcore::umaa::LargeListWriter<GlobalWaypointType, GlobalWaypointCommandTypeWaypointsListElement>
      listWriter(elementSender, waypoints);

  const arlcore::NumericGuid sessionId = arlcore::UuidFactory::getInstance().generateGuid();
  const arlcore::NumericGuid runnerId = arlcore::UuidFactory::getInstance().generateGuid();
  GlobalWaypointCommandType cmd;
  cmd.sessionID() = sessionId.getGuid();
  cmd.source().id() = runnerId.getGuid();
  cmd.destination().id() =
      arlcore::UuidFactory::getInstance().parseGuidFromString(config.identity.waypointSourceId).getGuid();
  cmd.timeStamp() = arlcore::umaa::getTimestamp();
  cmd.waypointsListMetadata() = listWriter.getMetadata();

  // Give discovery a moment so the transient-local route/command reach the autopilot together.
  std::this_thread::sleep_for(std::chrono::seconds(2));
  if (cmdSender->send(cmd) != arlcore::io::SendStatus::SUCCESS) {
    std::cerr << "Failed to publish waypoint command" << std::endl;
    return 1;
  }
  std::cout << "Mission command published (session " << sessionId << ", "
            << waypoints.size() << " waypoints)" << std::endl;

  std::ofstream track(outDir + "/track.csv");
  track.precision(10);
  track << "elapsed_s,lat_deg,lon_deg,yaw_rad,speed_mps\n";
  std::ofstream statusLog(outDir + "/status.log");

  const auto start = std::chrono::steady_clock::now();
  const auto deadline = start + std::chrono::minutes(20);
  double lastSpeed = 0.0;
  bool done = false;
  std::string finalStatus = "TIMEOUT";

  while (!done && std::chrono::steady_clock::now() < deadline) {
    const double elapsed =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();

    SpeedReportType speed;
    if (speedReader->readLatest(&speed) == ReadStatus::SUCCESS && speed.speedOverGround().has_value()) {
      lastSpeed = speed.speedOverGround().value();
    }
    GlobalPoseReportType pose;
    if (poseReader->readLatest(&pose) == ReadStatus::SUCCESS) {
      track << elapsed << "," << pose.position().geodeticLatitude() << ","
            << pose.position().geodeticLongitude() << "," << pose.attitude().yaw().yaw() << ","
            << lastSpeed << "\n";
    }

    GlobalWaypointCommandStatusType status;
    while (statusReader->read(&status) == ReadStatus::SUCCESS) {
      if (arlcore::NumericGuid(status.sessionID()) != sessionId) {
        continue;
      }
      const std::string name = statusName(status.commandStatus());
      std::cout << "[" << elapsed << "s] command status: " << name
                << " (" << status.logMessage() << ")" << std::endl;
      statusLog << elapsed << " " << name << " " << status.logMessage() << "\n";
      if (status.commandStatus() == CommandStatusEnumType::COMPLETED ||
          status.commandStatus() == CommandStatusEnumType::FAILED ||
          status.commandStatus() == CommandStatusEnumType::CANCELED) {
        finalStatus = name;
        done = true;
      }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  track.flush();
  statusLog << "final: " << finalStatus << "\n";
  std::cout << "Mission finished with status " << finalStatus << std::endl;
  return finalStatus == "COMPLETED" ? 0 : 2;
}
