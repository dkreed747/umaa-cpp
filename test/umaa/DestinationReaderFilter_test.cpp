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
#include <UMAA/MO/GlobalVectorControl/GlobalVectorCommandType.hpp>

#include "DestinationReaderFilter.h"
#include "CycloneReader.h"
#include "CycloneSender.h"
#include "UuidFactory.h"
#include "NumericGuid.h"
#include "Logger.h"

using UMAA::MO::GlobalVectorControl::GlobalVectorCommandType;

static const int32_t domainId = 83;
static const std::string commandTopic = UMAA::MO::GlobalVectorControl::GlobalVectorCommandTypeTopic;
static const arlcore::io::Duration maxWait = {0, 1000000}; // 0.001 seconds

static const arlcore::NumericGuid goodSourceId =
  arlcore::UuidFactory::getInstance().parseGuidFromString("00000000-0000-0000-0000-100000000000");
static const arlcore::NumericGuid badSourceId =
  arlcore::UuidFactory::getInstance().parseGuidFromString("00000000-0000-0000-0000-200000000000");

class DestinationReaderFilterTest : public ::testing::Test {
 protected:
  static void SetUpTestSuite() {
    participant_ = arlcore::io::getDomainParticipant(domainId);
    subscriber_ = arlcore::io::createSubscriber(participant_);
    publisher_ = arlcore::io::createPublisher(participant_);
    reader_ = std::make_shared<arlcore::io::CycloneReader<GlobalVectorCommandType>>(
        participant_, commandTopic, subscriber_.default_datareader_qos());
    sender_ = std::make_shared<arlcore::io::CycloneSender<GlobalVectorCommandType>>(
        participant_, commandTopic, publisher_.default_datawriter_qos());

    usleep(30000);  // Discovery action
  }
  void SetUp() override {
    // Code to run before running each TEST_F()
  }

  void TearDown() override {
    // Code to run after running each TEST_F()
  }
  static dds::domain::DomainParticipant participant_;
  static dds::pub::Publisher publisher_;
  static dds::sub::Subscriber subscriber_;
  static std::shared_ptr<arlcore::io::CycloneReader<GlobalVectorCommandType>> reader_;
  static std::shared_ptr<arlcore::io::CycloneSender<GlobalVectorCommandType>> sender_;
};

dds::domain::DomainParticipant DestinationReaderFilterTest::participant_ = dds::core::null;
dds::pub::Publisher DestinationReaderFilterTest::publisher_ = dds::core::null;
dds::sub::Subscriber DestinationReaderFilterTest::subscriber_ = dds::core::null;
std::shared_ptr<arlcore::io::CycloneReader<GlobalVectorCommandType>> DestinationReaderFilterTest::reader_(nullptr);
std::shared_ptr<arlcore::io::CycloneSender<GlobalVectorCommandType>> DestinationReaderFilterTest::sender_;

TEST_F(DestinationReaderFilterTest, TestFilter) {
  auto expDirectionMode = UMAA::Common::MaritimeEnumeration::DirectionModeEnumModule::DirectionModeEnumType::COURSE;
  flt64_t expDirection = 2.42;

  flt64_t expSpeedOverGround = 5.165;

  int32_t expDepthChangePitch = 142;
  int32_t expEndTime = 124;

  auto filter = std::make_shared<arlcore::umaa::DestinationReaderFilter<GlobalVectorCommandType>>(goodSourceId);
  reader_->setManualFilter(filter);

  GlobalVectorCommandType gvCommand, receivedCommand;
  UMAA::Common::IdentifierType dest(goodSourceId.getGuid(), arlcore::NIL_GUID.getGuid());
  gvCommand.destination(dest);

  // Sets Discriminator to DirectionTrueNorthRequirement
  gvCommand.direction().DirectionRequirementVariantTypeSubtypes().DirectionTrueNorthRequirementVariantVariant(UMAA::Common::Orientation::DirectionTrueNorthRequirementVariantType());
  gvCommand.direction().DirectionRequirementVariantTypeSubtypes().DirectionTrueNorthRequirementVariantVariant().direction().direction(expDirection);

  gvCommand.directionMode(expDirectionMode);

  gvCommand.speed().SpeedRequirementVariantTypeSubtypes().GroundSpeedRequirementVariantVariant(UMAA::Common::Speed::GroundSpeedRequirementVariantType());
  gvCommand.speed().SpeedRequirementVariantTypeSubtypes().GroundSpeedRequirementVariantVariant().speed().speed(expSpeedOverGround);

  UMAA::Common::Orientation::PitchYNEDRequirement dcp(UMAA::Common::Orientation::PitchYNEDType(expDepthChangePitch), UMAA::Common::Orientation::PitchYNEDTolerance(UMAA::Common::Measurement::DurationSeconds(0), UMAA::Common::Orientation::PitchYNEDType(0),UMAA::Common::Orientation::PitchYNEDType(0)));
  gvCommand.depthChangePitch(dcp);
  UMAA::Common::Measurement::DateTime dateTime(expEndTime, 0);

  gvCommand.endTime(dateTime);

  sender_->send(gvCommand);
  sender_->waitForAcknowledgements(maxWait);

  ASSERT_EQ(reader_->read(&receivedCommand), arlcore::io::ReadStatus::SUCCESS);
  EXPECT_EQ(receivedCommand, gvCommand);

  // Change to bad ID and resend
  gvCommand.destination().id(badSourceId.getGuid());
  sender_->send(gvCommand);
  sender_->waitForAcknowledgements(maxWait);

  ASSERT_EQ(reader_->read(&receivedCommand), arlcore::io::ReadStatus::NO_DATA);

  // Update the filter and resend
  filter->setFilterId(badSourceId);
  sender_->send(gvCommand);
  sender_->waitForAcknowledgements(maxWait);

  ASSERT_EQ(reader_->read(&receivedCommand), arlcore::io::ReadStatus::SUCCESS);
  EXPECT_EQ(receivedCommand, gvCommand);

}
