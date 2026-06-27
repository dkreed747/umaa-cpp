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

#include "CycloneQosProviderWrapper.h"
#include "Logger.h"

namespace arlcore::io {

  dds::domain::qos::DomainParticipantQos CycloneQosProviderWrapper::participant_qos(const std::string& id) {
    std::string profile = id.empty() ? qosProfile_ : id;

    try {
      auto qos = qosProvider_.participant_qos(profile);
      return qos;
    } catch (const std::exception& e) {
      // An infinite loop is caused if we use this class to send a message to the DDS bus before instantiating
      // the LogReport service.
      UMAA_LOG_TRACE(util::SYSTEM_LOGGER, "Using Default Domain Participant QoS.")
    }

    auto defaultQos = dds::domain::qos::DomainParticipantQos();
    return defaultQos;
  }

  dds::topic::qos::TopicQos CycloneQosProviderWrapper::topic_qos(const std::string& id) {
    std::string profile = id.empty() ? qosProfile_ : id;

    try {
      auto qos = qosProvider_.topic_qos(profile);
      return qos;
    } catch (const std::exception& e) {
      UMAA_LOG_TRACE(util::SYSTEM_LOGGER, "Using Default Topic QoS.")
    }

    auto defaultQos = dds::topic::qos::TopicQos();
    return defaultQos;
  }

  dds::pub::qos::PublisherQos CycloneQosProviderWrapper::publisher_qos(const std::string& id) {
    std::string profile = id.empty() ? qosProfile_ : id;

    try {
      auto qos = qosProvider_.publisher_qos(profile);
      return qos;
    } catch (const std::exception& e) {
      UMAA_LOG_TRACE(util::SYSTEM_LOGGER, "Using Default Publisher QoS.")
    }

    auto defaultQos = dds::pub::qos::PublisherQos();
    return defaultQos;
  }

  dds::sub::qos::SubscriberQos CycloneQosProviderWrapper::subscriber_qos(const std::string& id) {
    std::string profile = id.empty() ? qosProfile_ : id;

      try {
        auto qos = qosProvider_.subscriber_qos(profile);
        return qos;
      } catch (const std::exception& e) {
        UMAA_LOG_TRACE(util::SYSTEM_LOGGER, "Using Default Subscriber QoS.")
      }

    auto defaultQos = dds::sub::qos::SubscriberQos();
    return defaultQos;
  }

  dds::pub::qos::DataWriterQos CycloneQosProviderWrapper::datawriter_qos(const std::string& id) {
    std::string profile = id.empty() ? qosProfile_ : id;

      try {
        auto qos = qosProvider_.datawriter_qos(profile);
        return qos;
      } catch (const std::exception& e) {
        UMAA_LOG_TRACE(util::SYSTEM_LOGGER, "Using Default Datawriter QoS.")
      }

    auto defaultQos = dds::pub::qos::DataWriterQos();
    return defaultQos;
  }

  dds::sub::qos::DataReaderQos CycloneQosProviderWrapper::datareader_qos(const std::string& id) {
    std::string profile = id.empty() ? qosProfile_ : id;
      try {
        auto qos = qosProvider_.datareader_qos(profile);
        return qos;
      } catch (const std::exception& e) {
        UMAA_LOG_TRACE(util::SYSTEM_LOGGER, "Using Default Datareader QoS.")
      }

    auto defaultQos = dds::sub::qos::DataReaderQos();
    return defaultQos;
  }
}  // namespace arlcore::io
