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
#ifndef INCLUDE_IO_DDS_CYCLONE_CYCLONEQOSPROVIDERWRAPPER_H_
#define INCLUDE_IO_DDS_CYCLONE_CYCLONEQOSPROVIDERWRAPPER_H_

#include <string>
#include <memory>
#include <dds/dds.hpp>

namespace arlcore::io {

class CycloneQosProviderWrapper {
 public:
  CycloneQosProviderWrapper() = delete;

  CycloneQosProviderWrapper(const std::string& qosFile, const std::string& qosProfile) :
  qosProvider_(qosFile),
  qosProfile_(qosProfile) { }

  dds::domain::qos::DomainParticipantQos participant_qos(const std::string& id = "");
  dds::topic::qos::TopicQos topic_qos(const std::string& id = "");
  dds::pub::qos::PublisherQos publisher_qos(const std::string& id = "");
  dds::sub::qos::SubscriberQos subscriber_qos(const std::string& id = "");
  dds::pub::qos::DataWriterQos datawriter_qos(const std::string& id = "");
  dds::sub::qos::DataReaderQos datareader_qos(const std::string& id = "");

 private:
  dds::core::QosProvider qosProvider_;
  std::string qosProfile_;
};

}  // namespace arlcore::io

#endif  // INCLUDE_IO_DDS_CYCLONE_CYCLONEQOSPROVIDERWRAPPER_H_
