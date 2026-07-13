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

#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <dds/dds.hpp>

namespace arlcore::io {

//! \brief Resolves entity QoS from the DDS-XML profile file. Providers are scoped to one
//! "Library::Profile" at construction (the CycloneDDS QoS-provider model), so a provider is
//! created lazily per requested profile and cached. Any failure (missing file, bad XML,
//! unknown profile) falls back to default-constructed QoS so the system can still come up.
class CycloneQosProviderWrapper {
 public:
  CycloneQosProviderWrapper() = delete;

  CycloneQosProviderWrapper(const std::string& qosFile, const std::string& qosProfile) :
  qosFile_(qosFile),
  qosProfile_(qosProfile) { }

  //! \brief `id` selects a full "Library::Profile"; empty means the default profile from
  //! construction.
  dds::domain::qos::DomainParticipantQos participant_qos(const std::string& id = "");
  dds::topic::qos::TopicQos topic_qos(const std::string& id = "");
  dds::pub::qos::PublisherQos publisher_qos(const std::string& id = "");
  dds::sub::qos::SubscriberQos subscriber_qos(const std::string& id = "");
  dds::pub::qos::DataWriterQos datawriter_qos(const std::string& id = "");
  dds::sub::qos::DataReaderQos datareader_qos(const std::string& id = "");

 private:
  //! \brief The cached provider scoped to `profile`, or nullopt when it cannot be created.
  std::optional<dds::core::QosProvider> providerFor(const std::string& profile);

  std::string qosFile_;
  std::string qosProfile_;
  std::mutex mutex_;
  std::map<std::string, std::optional<dds::core::QosProvider>> providers_;
};

}  // namespace arlcore::io

#endif  // INCLUDE_IO_DDS_CYCLONE_CYCLONEQOSPROVIDERWRAPPER_H_
