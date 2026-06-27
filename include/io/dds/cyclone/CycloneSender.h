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

#ifndef INCLUDE_IO_DDS_CYCLONE_CYCLONESENDER_H_
#define INCLUDE_IO_DDS_CYCLONE_CYCLONESENDER_H_

#include <mutex>
#include <string>
#include <memory>

#include <dds/dds.hpp>

#include "SenderBase.h"
#include "CycloneUtilities.h"
#include "CycloneQosProviderWrapper.h"

namespace arlcore::io {


template <class DataType>
class CycloneSender : public SenderBase<DataType> {
 public:
  CycloneSender() = delete;

  //! \brief Standard Constructor for a Cyclone Sender object
  //! \param participant The DDS Domain Participant (one instance per app)
  //! \param topic       The string topic name to subscribe to
  //! \param qos         The DataWriter qos profile
  CycloneSender(const dds::domain::DomainParticipant& participant,
            const std::string& topic,
            const dds::pub::qos::DataWriterQos& qos) {
    auto ddsTopic = getTopic<DataType>(participant, topic);
    writer_ = dds::pub::DataWriter<DataType>(createPublisher(participant), ddsTopic, qos);
  }

  //! \brief Qos-Configurable Constructor for Cyclone Sender object
  //! \param participant The DDS Domain Participant (one instance per app)
  //! \param publisher   The DDS publisher
  //! \param topic       The string topic name to subscribe to
  //! \param writerQos   The DataWriter qos profile
  //! \param topicQos    The Topic qos profile
  CycloneSender(const dds::domain::DomainParticipant& participant,
            const dds::pub::Publisher& publisher,
            const std::string& topic,
            const dds::pub::qos::DataWriterQos& writerQos,
            const dds::topic::qos::TopicQos& topicQos) {
    auto ddsTopic = getTopic<DataType>(participant, topic, topicQos);
    writer_ = dds::pub::DataWriter<DataType>(publisher, ddsTopic, writerQos);
  }

  CycloneSender(const dds::domain::DomainParticipant& participant,
            const dds::pub::Publisher& publisher,
            const std::string& topic,
            const std::shared_ptr<arlcore::io::CycloneQosProviderWrapper>& wrapper,
            const std::string& profile = "") {
    auto ddsTopic = getTopic<DataType>(participant, topic, wrapper->topic_qos(profile));
    writer_ = dds::pub::DataWriter<DataType>(publisher, ddsTopic, wrapper->datawriter_qos(profile));
  }

  //! \brief Qos-Configurable Constructor for Cyclone Sender object
  //! \param participant The DDS Domain Participant (one instance per app)
  //! \param publisher   The DDS publisher
  //! \param topic       The string topic name to subscribe to
  //! \param writerQos   The DataWriter qos profile
  //! \param topicQos    The Topic qos profile
  CycloneSender(const dds::domain::DomainParticipant& participant,
            const dds::pub::Publisher& publisher,
            const std::string& topic,
            const dds::pub::qos::DataWriterQos& writerQos) {
    auto ddsTopic = getTopic<DataType>(participant, topic);
    writer_ = dds::pub::DataWriter<DataType>(publisher, ddsTopic, writerQos);
  }

  //! \brief Cyclone Implementation of the send interface function
  //! \param data The data to send
  //! \return     A SendStatus Enum indicating the results of the send function.
  SendStatus send(const DataType& data) override {
    std::lock_guard<std::mutex> lock(writerLock_);
    try {
      writer_.write(data);
    } catch (const std::exception& e) {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, e.what())
      return SendStatus::ERROR;
    }

    return SendStatus::SUCCESS;
  }

  //! \brief Cyclone Implementation of the dispose interface function
  //! \param data The instance to dispose (Primary key values must be populated)
  //! \return     A SendStatus Enum indicating the results of the dispose function.
  SendStatus dispose(const DataType& data) override {
    std::lock_guard<std::mutex> lock(writerLock_);
    auto instanceHandle = writer_.lookup_instance(data);

    if (instanceHandle == dds::core::InstanceHandle::nil()) {
      return SendStatus::ERROR;
    }
    try {
      writer_.dispose_instance(instanceHandle);
    } catch(const std::exception& e) {
      UMAA_LOG_ERROR(util::SYSTEM_LOGGER, e.what())
      return SendStatus::ERROR;
    }
    return SendStatus::SUCCESS;
  }

 private:
  dds::pub::DataWriter<DataType> writer_ = dds::core::null;
  std::mutex writerLock_;
};

}  // namespace arlcore::io
#endif  // INCLUDE_IO_DDS_CYCLONE_CYCLONESENDER_H_
