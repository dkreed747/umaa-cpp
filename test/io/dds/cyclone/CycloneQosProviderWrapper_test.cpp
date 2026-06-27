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

#include <gtest/gtest.h>
#include <stdlib.h>

#include "CycloneQosProviderWrapper.h"


class CycloneQosProviderTest : public ::testing::Test {
 protected:
  void SetUp() override {
    provider_ = std::make_shared<arlcore::io::CycloneQosProviderWrapper>(TEST_QOS_FILE, PROFILE);
  }

  std::shared_ptr<arlcore::io::CycloneQosProviderWrapper> provider_;
  // Placeholders for expected QoS's - to be configured in unit tests below.
  dds::domain::qos::DomainParticipantQos expPQos;
  dds::topic::qos::TopicQos expTQos;
  dds::pub::qos::PublisherQos expPubQos;
  dds::sub::qos::SubscriberQos expSubQos;
  dds::pub::qos::DataWriterQos expDwQos;
  dds::sub::qos::DataReaderQos expDrQos;
  const std::string TEST_QOS_FILE = "TEST_QOS_PROFILES.xml";
  const std::string PROFILE = "UMAA_QoS_Library::DefaultQosProfile";
  const std::string EMPTY_PROFILE = "UMAA_QoS_Library::EmptyProfile";
};

TEST_F(CycloneQosProviderTest, testDefault) {
  ASSERT_EQ(provider_->participant_qos(), provider_->participant_qos(PROFILE));
  ASSERT_EQ(provider_->topic_qos(), provider_->topic_qos(PROFILE));
  ASSERT_EQ(provider_->publisher_qos(), provider_->publisher_qos(PROFILE));
  ASSERT_EQ(provider_->subscriber_qos(), provider_->subscriber_qos(PROFILE));
  ASSERT_EQ(provider_->datawriter_qos(), provider_->datawriter_qos(PROFILE));
  ASSERT_EQ(provider_->datareader_qos(),provider_->datareader_qos(PROFILE));

}

TEST_F(CycloneQosProviderTest, testEmptyNode) {
  ASSERT_EQ(provider_->participant_qos(EMPTY_PROFILE), expPQos);
  ASSERT_EQ(provider_->topic_qos(EMPTY_PROFILE), expTQos);
  ASSERT_EQ(provider_->publisher_qos(EMPTY_PROFILE), expPubQos);
  ASSERT_EQ(provider_->subscriber_qos(EMPTY_PROFILE), expSubQos);
  ASSERT_EQ(provider_->datawriter_qos(EMPTY_PROFILE), expDwQos);
  ASSERT_EQ(provider_->datareader_qos(EMPTY_PROFILE),expDrQos);
}
