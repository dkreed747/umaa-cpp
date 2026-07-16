//---------------------------------------------------------------------------
// Copyright 2026 Pennsylvania State University
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

#include "ConditionalAddProvider.h"
#include "LocalReaderSender.h"
#include "UuidFactory.h"

using arlcore::io::LocalReaderSender;
using arlcore::io::ReadStatus;
using arlcore::io::SendStatus;

using UMAA::SA::GlobalPoseStatus::GlobalPoseReportType;
using UMAA::SA::SpeedStatus::SpeedReportType;
using UMAA::SA::VelocityStatus::VelocityReportType;
using UMAA::MM::ConditionalReport::ConditionalReportType;
using UMAA::MM::ConditionalReport::ConditionalReportTypeConditionalsSetElement;
using UMAA::MM::Conditional::ConditionalType;
using UMAA::MM::Conditional::DepthConditionalType;
using UMAA::MM::Conditional::SpeedConditionalType;
using UMAA::Common::MaritimeEnumeration::ConditionalOperatorEnumModule::ConditionalOperatorEnumType;
using UMAA::Common::Measurement::DateTime;

class ConditionalAddProviderTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Console-side payload topics (the commander publishes specialization payloads here; the factory io caches
    // read them back)
    consoleDepthRw_ = std::make_shared<LocalReaderSender<DepthConditionalType>>();
    consoleSpeedRw_ = std::make_shared<LocalReaderSender<SpeedConditionalType>>();

    // Autopilot-side report topics (the report provider re-publishes owned payload instances here)
    reportRw_ = std::make_shared<LocalReaderSender<ConditionalReportType>>();
    setElementRw_ = std::make_shared<LocalReaderSender<ConditionalReportTypeConditionalsSetElement>>();
    ownedDepthRw_ = std::make_shared<LocalReaderSender<DepthConditionalType>>();

    auto reportIo = std::make_shared<arlcore::umaa::ConditionalReportProviderIo>(
      reportRw_, setElementRw_,
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::ConstraintViolatedConditionalType>>(),
      ownedDepthRw_,
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::DepthRateConditionalType>>(),
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::EmitterPresetConditionalType>>(),
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::ExpConditionalType>>(),
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::HeadingSectorConditionalType>>(),
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::LogicalANDConditionalType>>(),
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::LogicalNOTConditionalType>>(),
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::LogicalORConditionalType>>(),
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::MissionStateConditionalType>>(),
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::ObjectiveStateConditionalType>>(),
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::PitchRateConditionalType>>(),
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::RelativeSpeedConditionalType>>(),
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::RollRateConditionalType>>(),
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::SpeedConditionalType>>(),
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::TaskStateConditionalType>>(),
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::TimeConditionalType>>(),
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::WaterZoneConditionalType>>(),
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::YawRateConditionalType>>());
    reportProvider_ = std::make_shared<arlcore::umaa::conditional::ConditionalReportProvider>(
      arlcore::UuidFactory::getInstance().generateGuid(), reportIo);

    auto gpConsumer = std::make_shared<arlcore::umaa::services::GlobalPoseReportConsumer>(
      std::make_shared<LocalReaderSender<GlobalPoseReportType>>());
    auto ssConsumer = std::make_shared<arlcore::umaa::services::SpeedReportConsumer>(
      std::make_shared<LocalReaderSender<SpeedReportType>>());
    auto vsConsumer = std::make_shared<arlcore::umaa::services::VelocityReportConsumer>(
      std::make_shared<LocalReaderSender<VelocityReportType>>());
    factoryIo_ = std::make_shared<arlcore::umaa::ConditionalFactoryIo>(gpConsumer, ssConsumer, vsConsumer,
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::ConstraintViolatedConditionalType>>(),
      consoleDepthRw_,
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::DepthRateConditionalType>>(),
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::EmitterPresetConditionalType>>(),
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::ExpConditionalType>>(),
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::HeadingSectorConditionalType>>(),
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::LogicalANDConditionalType>>(),
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::LogicalNOTConditionalType>>(),
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::LogicalORConditionalType>>(),
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::MissionStateConditionalType>>(),
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::ObjectiveStateConditionalType>>(),
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::PitchRateConditionalType>>(),
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::RelativeSpeedConditionalType>>(),
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::RollRateConditionalType>>(),
      consoleSpeedRw_,
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::TaskStateConditionalType>>(),
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::TimeConditionalType>>(),
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::WaterZoneConditionalType>>(),
      std::make_shared<LocalReaderSender<UMAA::MM::Conditional::YawRateConditionalType>>());

    cmdRw_ = std::make_shared<LocalReaderSender<ConditionalAddCommandType>>();
    ackRw_ = std::make_shared<LocalReaderSender<ConditionalAddCommandAckReportType>>();
    statusRw_ = std::make_shared<LocalReaderSender<ConditionalAddCommandStatusType>>();
    addIo_ = std::make_shared<ConditionalAddProviderIo>(cmdRw_, ackRw_, statusRw_);
    source_ = arlcore::UuidFactory::getInstance().generateGuid();
  }

  //! Build a console-side generic + payload pair for a depth conditional and publish the payload
  ConditionalType publishDepthPayload(const arlcore::NumericGuid& conditionalId, flt64_t depth,
      int64_t timeSeconds) {
    DateTime stamp(timeSeconds, 0);
    arlcore::NumericGuid specId = arlcore::UuidFactory::getInstance().generateGuid();
    DepthConditionalType payload(ConditionalOperatorEnumType::LESS_THAN, depth, stamp, specId.getGuid());
    EXPECT_EQ(consoleDepthRw_->send(payload), SendStatus::SUCCESS);

    ConditionalType generic;
    generic.conditionalID(conditionalId.getGuid());
    generic.name("depth-limit");
    generic.specializationTopic(UMAA::MM::Conditional::DepthConditionalTypeTopic);
    generic.specializationID(specId.getGuid());
    generic.specializationTimestamp(stamp);
    return generic;
  }

  ConditionalAddCommandType makeCommand(const ConditionalType& conditional) {
    ConditionalAddCommandType cmd;
    cmd.conditional(conditional);
    cmd.sessionID(arlcore::UuidFactory::getInstance().generateGuid().getGuid());
    cmd.destination().id(source_.getGuid());
    return cmd;
  }

  void expectStatus(CommandStatusEnumType status,
      CommandStatusReasonEnumType reason = CommandStatusReasonEnumType::SUCCEEDED) {
    ConditionalAddCommandStatusType sample;
    ASSERT_EQ(statusRw_->read(&sample), ReadStatus::SUCCESS);
    EXPECT_EQ(sample.commandStatus(), status);
    EXPECT_EQ(sample.commandStatusReason(), reason);
  }

  arlcore::NumericGuid source_;
  std::shared_ptr<LocalReaderSender<DepthConditionalType>> consoleDepthRw_;
  std::shared_ptr<LocalReaderSender<SpeedConditionalType>> consoleSpeedRw_;
  std::shared_ptr<LocalReaderSender<ConditionalReportType>> reportRw_;
  std::shared_ptr<LocalReaderSender<ConditionalReportTypeConditionalsSetElement>> setElementRw_;
  std::shared_ptr<LocalReaderSender<DepthConditionalType>> ownedDepthRw_;
  std::shared_ptr<LocalReaderSender<ConditionalAddCommandType>> cmdRw_;
  std::shared_ptr<LocalReaderSender<ConditionalAddCommandAckReportType>> ackRw_;
  std::shared_ptr<LocalReaderSender<ConditionalAddCommandStatusType>> statusRw_;
  std::shared_ptr<ConditionalAddProviderIo> addIo_;
  std::shared_ptr<arlcore::umaa::conditional::ConditionalReportProvider> reportProvider_;
  std::shared_ptr<arlcore::umaa::ConditionalFactoryIo> factoryIo_;
};

TEST_F(ConditionalAddProviderTest, addPublishesOwnedPayloadAndPreservesConditionalId) {
  arlcore::umaa::conditional::ConditionalAddProvider provider(source_, addIo_, reportProvider_, factoryIo_);

  arlcore::NumericGuid conditionalId = arlcore::UuidFactory::getInstance().generateGuid();
  ConditionalType requested = publishDepthPayload(conditionalId, 25.0, 100);
  EXPECT_EQ(cmdRw_->send(makeCommand(requested)), SendStatus::SUCCESS);

  EXPECT_TRUE(provider.cycle());

  expectStatus(CommandStatusEnumType::ISSUED);
  expectStatus(CommandStatusEnumType::COMMANDED);
  expectStatus(CommandStatusEnumType::EXECUTING);
  expectStatus(CommandStatusEnumType::COMPLETED);

  // The report preserves the commander-minted conditionalID under a fresh, provider-owned specializationID
  std::optional<ConditionalType> stored = reportProvider_->getConditionalById(conditionalId);
  ASSERT_TRUE(stored.has_value());
  EXPECT_EQ(stored->name(), requested.name());
  EXPECT_NE(arlcore::NumericGuid(stored->specializationID()), arlcore::NumericGuid(requested.specializationID()));

  // The payload was re-published under the report provider's writer with matching metadata and content
  DepthConditionalType owned;
  ASSERT_EQ(ownedDepthRw_->read(&owned), ReadStatus::SUCCESS);
  EXPECT_EQ(arlcore::NumericGuid(owned.specializationReferenceID()), arlcore::NumericGuid(stored->specializationID()));
  EXPECT_EQ(owned.specializationReferenceTimestamp(), stored->specializationTimestamp());
  EXPECT_EQ(owned.depth(), 25.0);

  // The updated report went out
  ConditionalReportType report;
  EXPECT_EQ(reportRw_->read(&report), ReadStatus::SUCCESS);
}

TEST_F(ConditionalAddProviderTest, addWaitsForLatePayload) {
  arlcore::umaa::conditional::ConditionalAddProvider provider(source_, addIo_, reportProvider_, factoryIo_);

  // Build the generic + payload, but hold the payload back
  DateTime stamp(200, 0);
  arlcore::NumericGuid conditionalId = arlcore::UuidFactory::getInstance().generateGuid();
  arlcore::NumericGuid specId = arlcore::UuidFactory::getInstance().generateGuid();
  ConditionalType requested;
  requested.conditionalID(conditionalId.getGuid());
  requested.specializationTopic(UMAA::MM::Conditional::DepthConditionalTypeTopic);
  requested.specializationID(specId.getGuid());
  requested.specializationTimestamp(stamp);
  EXPECT_EQ(cmdRw_->send(makeCommand(requested)), SendStatus::SUCCESS);

  EXPECT_TRUE(provider.cycle());
  EXPECT_TRUE(provider.cycle());
  EXPECT_EQ(provider.getCommandStatus().value(), CommandStatusEnumType::EXECUTING);
  EXPECT_FALSE(reportProvider_->getConditionalById(conditionalId).has_value());

  // Payload arrives late; the command completes on the next cycle
  DepthConditionalType payload(ConditionalOperatorEnumType::LESS_THAN, 10.0, stamp, specId.getGuid());
  EXPECT_EQ(consoleDepthRw_->send(payload), SendStatus::SUCCESS);
  EXPECT_TRUE(provider.cycle());

  EXPECT_TRUE(reportProvider_->getConditionalById(conditionalId).has_value());
  expectStatus(CommandStatusEnumType::ISSUED);
  expectStatus(CommandStatusEnumType::COMMANDED);
  expectStatus(CommandStatusEnumType::EXECUTING);
  expectStatus(CommandStatusEnumType::COMPLETED);
}

TEST_F(ConditionalAddProviderTest, addTimesOutWhenPayloadNeverArrives) {
  arlcore::umaa::conditional::ConditionalAddProvider provider(source_, addIo_, reportProvider_, factoryIo_, {}, 2);

  ConditionalType requested;
  requested.conditionalID(arlcore::UuidFactory::getInstance().generateGuid().getGuid());
  requested.specializationTopic(UMAA::MM::Conditional::DepthConditionalTypeTopic);
  requested.specializationID(arlcore::UuidFactory::getInstance().generateGuid().getGuid());
  requested.specializationTimestamp(DateTime(300, 0));
  EXPECT_EQ(cmdRw_->send(makeCommand(requested)), SendStatus::SUCCESS);

  EXPECT_TRUE(provider.cycle());  // waitCycles = 1
  EXPECT_TRUE(provider.cycle());  // waitCycles = 2
  EXPECT_TRUE(provider.cycle());  // budget exceeded -> TIMEOUT

  expectStatus(CommandStatusEnumType::ISSUED);
  expectStatus(CommandStatusEnumType::COMMANDED);
  expectStatus(CommandStatusEnumType::EXECUTING);
  expectStatus(CommandStatusEnumType::FAILED, CommandStatusReasonEnumType::TIMEOUT);
}

TEST_F(ConditionalAddProviderTest, addRejectsUnsupportedTopic) {
  arlcore::umaa::conditional::ConditionalAddProvider provider(source_, addIo_, reportProvider_, factoryIo_,
    {UMAA::MM::Conditional::DepthConditionalTypeTopic});

  ConditionalType requested;
  requested.conditionalID(arlcore::UuidFactory::getInstance().generateGuid().getGuid());
  requested.specializationTopic(UMAA::MM::Conditional::SpeedConditionalTypeTopic);
  requested.specializationID(arlcore::UuidFactory::getInstance().generateGuid().getGuid());
  EXPECT_EQ(cmdRw_->send(makeCommand(requested)), SendStatus::SUCCESS);

  EXPECT_TRUE(provider.cycle());

  expectStatus(CommandStatusEnumType::ISSUED);
  expectStatus(CommandStatusEnumType::FAILED, CommandStatusReasonEnumType::VALIDATION_FAILED);
}

TEST_F(ConditionalAddProviderTest, addRejectsNilConditionalId) {
  arlcore::umaa::conditional::ConditionalAddProvider provider(source_, addIo_, reportProvider_, factoryIo_);

  ConditionalType requested;
  requested.specializationTopic(UMAA::MM::Conditional::DepthConditionalTypeTopic);
  requested.specializationID(arlcore::UuidFactory::getInstance().generateGuid().getGuid());
  EXPECT_EQ(cmdRw_->send(makeCommand(requested)), SendStatus::SUCCESS);

  EXPECT_TRUE(provider.cycle());

  expectStatus(CommandStatusEnumType::ISSUED);
  expectStatus(CommandStatusEnumType::FAILED, CommandStatusReasonEnumType::VALIDATION_FAILED);
}

TEST_F(ConditionalAddProviderTest, addWithExistingConditionalIdIsUpsert) {
  arlcore::umaa::conditional::ConditionalAddProvider provider(source_, addIo_, reportProvider_, factoryIo_);

  arlcore::NumericGuid conditionalId = arlcore::UuidFactory::getInstance().generateGuid();
  ConditionalType first = publishDepthPayload(conditionalId, 25.0, 100);
  EXPECT_EQ(cmdRw_->send(makeCommand(first)), SendStatus::SUCCESS);
  EXPECT_TRUE(provider.cycle());

  std::optional<ConditionalType> initial = reportProvider_->getConditionalById(conditionalId);
  ASSERT_TRUE(initial.has_value());
  DepthConditionalType firstOwned;
  ASSERT_EQ(ownedDepthRw_->read(&firstOwned), ReadStatus::SUCCESS);

  // Second Add with the same conditionalID but a fresh payload (the commander edited the constraint)
  ConditionalType second = publishDepthPayload(conditionalId, 15.0, 150);
  EXPECT_EQ(cmdRw_->send(makeCommand(second)), SendStatus::SUCCESS);
  EXPECT_TRUE(provider.cycle());

  // Still a single conditional under the same ID, now on a new owned payload instance
  EXPECT_EQ(reportProvider_->getConditionals().size(), 1);
  std::optional<ConditionalType> updated = reportProvider_->getConditionalById(conditionalId);
  ASSERT_TRUE(updated.has_value());
  EXPECT_NE(arlcore::NumericGuid(updated->specializationID()), arlcore::NumericGuid(initial->specializationID()));

  DepthConditionalType secondOwned;
  ASSERT_EQ(ownedDepthRw_->read(&secondOwned), ReadStatus::SUCCESS);
  EXPECT_EQ(secondOwned.depth(), 15.0);
  EXPECT_EQ(arlcore::NumericGuid(secondOwned.specializationReferenceID()),
    arlcore::NumericGuid(updated->specializationID()));

  // The superseded owned instance was disposed
  DepthConditionalType disposed;
  ASSERT_EQ(ownedDepthRw_->read(&disposed), ReadStatus::DISPOSED);
  EXPECT_EQ(arlcore::NumericGuid(disposed.specializationReferenceID()),
    arlcore::NumericGuid(initial->specializationID()));
}
