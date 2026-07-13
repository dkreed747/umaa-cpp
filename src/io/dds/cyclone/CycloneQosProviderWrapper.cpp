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

#include <string>

#include "Logger.h"

namespace arlcore::io {

  std::optional<dds::core::QosProvider> CycloneQosProviderWrapper::providerFor(
      const std::string& profile) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = providers_.find(profile);
    if (it != providers_.end()) {
      return it->second;
    }
    std::optional<dds::core::QosProvider> provider;
    try {
      // A provider is scoped to one Library::Profile; entity lookups are relative to it.
      provider.emplace(qosFile_, profile);
    } catch (const std::exception& e) {
      // Note TRACE, not ERROR: logging to the DDS bus from here would recurse into the
      // log service before its own QoS provider exists.
      UMAA_LOG_TRACE(util::SYSTEM_LOGGER, "QoS profile '" << profile << "' unavailable from '"
        << qosFile_ << "' (" << e.what() << "); using default QoS.")
    }
    providers_.emplace(profile, provider);
    return provider;
  }

  dds::domain::qos::DomainParticipantQos CycloneQosProviderWrapper::participant_qos(const std::string& id) {
    try {
      auto provider = providerFor(id.empty() ? qosProfile_ : id);
      if (provider.has_value()) {
        return provider->participant_qos();
      }
    } catch (const std::exception& e) {
      UMAA_LOG_TRACE(util::SYSTEM_LOGGER, "Using Default Domain Participant QoS.")
    }
    return dds::domain::qos::DomainParticipantQos();
  }

  dds::topic::qos::TopicQos CycloneQosProviderWrapper::topic_qos(const std::string& id) {
    try {
      auto provider = providerFor(id.empty() ? qosProfile_ : id);
      if (provider.has_value()) {
        return provider->topic_qos();
      }
    } catch (const std::exception& e) {
      UMAA_LOG_TRACE(util::SYSTEM_LOGGER, "Using Default Topic QoS.")
    }
    return dds::topic::qos::TopicQos();
  }

  dds::pub::qos::PublisherQos CycloneQosProviderWrapper::publisher_qos(const std::string& id) {
    try {
      auto provider = providerFor(id.empty() ? qosProfile_ : id);
      if (provider.has_value()) {
        return provider->publisher_qos();
      }
    } catch (const std::exception& e) {
      UMAA_LOG_TRACE(util::SYSTEM_LOGGER, "Using Default Publisher QoS.")
    }
    return dds::pub::qos::PublisherQos();
  }

  dds::sub::qos::SubscriberQos CycloneQosProviderWrapper::subscriber_qos(const std::string& id) {
    try {
      auto provider = providerFor(id.empty() ? qosProfile_ : id);
      if (provider.has_value()) {
        return provider->subscriber_qos();
      }
    } catch (const std::exception& e) {
      UMAA_LOG_TRACE(util::SYSTEM_LOGGER, "Using Default Subscriber QoS.")
    }
    return dds::sub::qos::SubscriberQos();
  }

  dds::pub::qos::DataWriterQos CycloneQosProviderWrapper::datawriter_qos(const std::string& id) {
    try {
      auto provider = providerFor(id.empty() ? qosProfile_ : id);
      if (provider.has_value()) {
        return provider->datawriter_qos();
      }
    } catch (const std::exception& e) {
      UMAA_LOG_TRACE(util::SYSTEM_LOGGER, "Using Default Datawriter QoS.")
    }
    return dds::pub::qos::DataWriterQos();
  }

  dds::sub::qos::DataReaderQos CycloneQosProviderWrapper::datareader_qos(const std::string& id) {
    try {
      auto provider = providerFor(id.empty() ? qosProfile_ : id);
      if (provider.has_value()) {
        return provider->datareader_qos();
      }
    } catch (const std::exception& e) {
      UMAA_LOG_TRACE(util::SYSTEM_LOGGER, "Using Default Datareader QoS.")
    }
    return dds::sub::qos::DataReaderQos();
  }
}  // namespace arlcore::io
