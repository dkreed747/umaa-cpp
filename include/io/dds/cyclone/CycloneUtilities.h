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

#ifndef INCLUDE_IO_DDS_CYCLONE_CYCLONEUTILITIES_H_
#define INCLUDE_IO_DDS_CYCLONE_CYCLONEUTILITIES_H_

#include <string>

#include <dds/dds.hpp>

namespace arlcore::io {


//! Retrieves a previously-created DomainParticipant belonging to the specific domainId
//! If no DomainParticipant exists, creates and returns a DomainParticipant with the given
//! QoS settings.
//! \param domainId       The domainId to search
//! \param participantQos The DomainParticipant QoS
//! \return A DomainParticipant
static dds::domain::DomainParticipant getDomainParticipant(
    const int32_t& domainId,
    const dds::domain::qos::DomainParticipantQos& participantQos) {
  dds::domain::DomainParticipant retParticipant = dds::domain::find(domainId);
  if (retParticipant == dds::core::null) {
    retParticipant = dds::domain::DomainParticipant(domainId, participantQos);
  }
  return retParticipant;
}

//! Retrieves a previously-created DomainParticipant belonging to the specific domainId
//! If no DomainParticipant exists, creates and returns a DomainParticipant built with
//! default QoS settings.
//! \param domainId       The domainId to search
//! \return A DomainParticipant
static dds::domain::DomainParticipant getDomainParticipant(const int32_t& domainId) {
    dds::domain::DomainParticipant retParticipant = dds::domain::find(domainId);

  if (retParticipant == dds::core::null) {
    retParticipant = dds::domain::DomainParticipant(domainId);
  }
  return retParticipant;
}

//! Creates a subscriber attached to the given DomainParticipant.
//! \param participant The DomainParticipant that will own this subscriber.
static dds::sub::Subscriber createSubscriber(
    const dds::domain::DomainParticipant& participant) {
  dds::sub::Subscriber sub(participant);
  return sub;
}

//! Creates a publisher attached to the given DomainParticipant.
//! \param participant The DomainParticipant that will own this subscriber.
static dds::pub::Publisher createPublisher(
    const dds::domain::DomainParticipant& participant) {
  dds::pub::Publisher pub(participant);
  return pub;
}

//! Finds the Topic corresponding to the DomainParticipant and TopicName.
//! If none is found, creates a new Topic.
//! \param participant  The DomainParticipant to find the topic on
//! \param topicName    The topic name to find
//! \return A reference to a Topic proxy that exists locally.
template<class DataType>
static dds::topic::Topic<DataType> getTopic(
    const dds::domain::DomainParticipant& participant,
    const std::string& topicName) {

  dds::topic::Topic<DataType> retTopic = dds::topic::find<dds::topic::Topic<DataType>>(participant, topicName);

  if (retTopic == dds::core::null) {
    retTopic = dds::topic::Topic<DataType>(participant, topicName);
  }
  return retTopic;
}

//! Finds the Topic corresponding to the DomainParticipant and TopicName.
//! If none is found, creates a new Topic with a specified TopicQos.
//! \param participant  The DomainParticipant to find the topic on
//! \param topicName    The topic name to find
//! \param topicQos     The topic QoS
//! \return A reference to a Topic proxy that exists locally.
template<class DataType>
static dds::topic::Topic<DataType> getTopic(
    const dds::domain::DomainParticipant& participant,
    const std::string& topicName,
    const dds::topic::qos::TopicQos& topicQos) {

  dds::topic::Topic<DataType> retTopic = dds::topic::find<dds::topic::Topic<DataType>>(participant, topicName);

  if (retTopic == dds::core::null) {
    retTopic = dds::topic::Topic<DataType>(participant, topicName, topicQos);
  }
  return retTopic;
}
}  // namespace arlcore::io
#endif  // INCLUDE_IO_DDS_CYCLONE_CYCLONEUTILITIES_H_
