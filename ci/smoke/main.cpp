//---------------------------------------------------------------------------
// Smoke test for the installed umaa-cpp package: publish and receive a UMAA
// report through the SDK's Cyclone reader/sender over loopback DDS.
// Exits 0 when the sample round-trips, non-zero otherwise.
//---------------------------------------------------------------------------

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include <dds/dds.hpp>

#include "CycloneReader.h"
#include "CycloneSender.h"
#include "UMAA/SA/GlobalPoseStatus/GlobalPoseReportType.hpp"

int main() {
  using ReportType = UMAA::SA::GlobalPoseStatus::GlobalPoseReportType;
  const std::string topic = "umaa_cpp_smoke_global_pose";

  dds::domain::DomainParticipant participant(0);

  arlcore::io::CycloneReader<ReportType> reader(
      participant, topic, dds::sub::qos::DataReaderQos());
  arlcore::io::CycloneSender<ReportType> sender(
      participant, topic, dds::pub::qos::DataWriterQos());

  ReportType out{};

  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(20);
  while (std::chrono::steady_clock::now() < deadline) {
    if (sender.send(out) != arlcore::io::SendStatus::SUCCESS) {
      std::cerr << "umaa-cpp-smoke: send failed" << std::endl;
      return 2;
    }

    ReportType in{};
    if (reader.read(&in) == arlcore::io::ReadStatus::SUCCESS) {
      std::cout << "umaa-cpp-smoke: OK (sample received over loopback)" << std::endl;
      return 0;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }

  std::cerr << "umaa-cpp-smoke: timed out waiting for sample" << std::endl;
  return 1;
}
