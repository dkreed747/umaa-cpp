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

#include "ActiveConstraintsControlProvider.h"
#include "ConditionalReportConsumer.h"
#include "ConditionalFactory.h"
#include "LocalReaderSender.h"
#include "UuidFactory.h"
#include "LargeSetWriter.h"

using arlcore::io::LocalReaderSender;

using UMAA::SA::GlobalPoseStatus::GlobalPoseReportType;
using UMAA::SA::SpeedStatus::SpeedReportType;
using UMAA::SA::VelocityStatus::VelocityReportType;
using UMAA::MM::ConditionalReport::ConditionalReportType;
using UMAA::MM::ConditionalReport::ConditionalReportTypeConditionalsSetElement;
using UMAA::MM::Conditional::ConditionalType;
using UMAA::MM::Conditional::ConstraintViolatedConditionalType;
using UMAA::MM::Conditional::DepthConditionalType;
using UMAA::MM::Conditional::DepthRateConditionalType;
using UMAA::MM::Conditional::EmitterPresetConditionalType;
using UMAA::MM::Conditional::ExpConditionalType;
using UMAA::MM::Conditional::HeadingSectorConditionalType;
using UMAA::MM::Conditional::LogicalANDConditionalType;
using UMAA::MM::Conditional::LogicalNOTConditionalType;
using UMAA::MM::Conditional::LogicalORConditionalType;
using UMAA::MM::Conditional::MissionStateConditionalType;
using UMAA::MM::Conditional::ObjectiveStateConditionalType;
using UMAA::MM::Conditional::PitchRateConditionalType;
using UMAA::MM::Conditional::RelativeSpeedConditionalType;
using UMAA::MM::Conditional::RollRateConditionalType;
using UMAA::MM::Conditional::SpeedConditionalType;
using UMAA::MM::Conditional::TaskStateConditionalType;
using UMAA::MM::Conditional::TimeConditionalType;
using UMAA::MM::Conditional::WaterZoneConditionalType;
using UMAA::MM::Conditional::YawRateConditionalType;

using UMAA::Common::MaritimeEnumeration::ConditionalOperatorEnumModule::ConditionalOperatorEnumType;

class ActiveConstraintsControlProviderTest : public ::testing::Test {
 protected:

  static void SetUpTestSuite() {
    globalPoseReader_ = std::make_shared<LocalReaderSender<GlobalPoseReportType>>();
    speedStatusReader_ = std::make_shared<LocalReaderSender<SpeedReportType>>();
    velocityReportReader_ = std::make_shared<LocalReaderSender<VelocityReportType>>();
    activeConstraintsCommandReader_ = std::make_shared<LocalReaderSender<ActiveConstraintsCommandType>>();
    activeConstraintsCommandAckReportReader_ = std::make_shared<LocalReaderSender<ActiveConstraintsCommandAckReportType>>();
    activeConstraintsCommandStatusReader_ = std::make_shared<LocalReaderSender<ActiveConstraintsCommandStatusType>>();
    reportReader_ = std::make_shared<LocalReaderSender<ConditionalReportType>>();
    conditionalSetReader_ = std::make_shared<LocalReaderSender<ConditionalReportTypeConditionalsSetElement>>();
    constraintViolatedConditionalReader_ = std::make_shared<LocalReaderSender<ConstraintViolatedConditionalType>>();
    depthConditionalReader_ = std::make_shared<LocalReaderSender<DepthConditionalType>>();
    depthRateConditionalReader_ = std::make_shared<LocalReaderSender<DepthRateConditionalType>>();
    emitterPresetConditionalReader_ = std::make_shared<LocalReaderSender<EmitterPresetConditionalType>>();
    expConditionalReader_ = std::make_shared<LocalReaderSender<ExpConditionalType>>();
    headingSectorConditionalReader_ = std::make_shared<LocalReaderSender<HeadingSectorConditionalType>>();
    logicalANDConditionalReader_ = std::make_shared<LocalReaderSender<LogicalANDConditionalType>>();
    logicalNOTConditionalReader_ = std::make_shared<LocalReaderSender<LogicalNOTConditionalType>>();
    logicalORConditionalReader_ = std::make_shared<LocalReaderSender<LogicalORConditionalType>>();
    missionStateConditionalReader_ = std::make_shared<LocalReaderSender<MissionStateConditionalType>>();
    objectiveStateConditionalReader_ = std::make_shared<LocalReaderSender<ObjectiveStateConditionalType>>();
    pitchRateConditionalReader_ = std::make_shared<LocalReaderSender<PitchRateConditionalType>>();
    relativeSpeedConditionalReader_ = std::make_shared<LocalReaderSender<RelativeSpeedConditionalType>>();
    rollRateConditionalReader_ = std::make_shared<LocalReaderSender<RollRateConditionalType>>();
    speedConditionalReader_ = std::make_shared<LocalReaderSender<SpeedConditionalType>>();
    taskStateConditionalReader_ = std::make_shared<LocalReaderSender<TaskStateConditionalType>>();
    timeConditionalReader_ = std::make_shared<LocalReaderSender<TimeConditionalType>>();
    waterZoneConditionalReader_ = std::make_shared<LocalReaderSender<WaterZoneConditionalType>>();
    yawRateConditionalReader_ = std::make_shared<LocalReaderSender<YawRateConditionalType>>();
    auto gpReportConsumer = std::make_shared<arlcore::umaa::services::GlobalPoseReportConsumer>(globalPoseReader_);
    auto ssReportConsumer = std::make_shared<arlcore::umaa::services::SpeedReportConsumer>(speedStatusReader_);
    auto vsReportConsumer = std::make_shared<arlcore::umaa::services::VelocityReportConsumer>(velocityReportReader_);
    conditionalIo_ = std::make_shared<arlcore::umaa::ConditionalFactoryIo>(gpReportConsumer, ssReportConsumer, vsReportConsumer, constraintViolatedConditionalReader_, depthConditionalReader_,
        depthRateConditionalReader_, emitterPresetConditionalReader_, expConditionalReader_, headingSectorConditionalReader_, logicalANDConditionalReader_,
        logicalNOTConditionalReader_, logicalORConditionalReader_, missionStateConditionalReader_, objectiveStateConditionalReader_, pitchRateConditionalReader_,
        relativeSpeedConditionalReader_, rollRateConditionalReader_, speedConditionalReader_, taskStateConditionalReader_, timeConditionalReader_,
        waterZoneConditionalReader_, yawRateConditionalReader_);
    constraintsIo_ = std::make_shared<ActiveConstraintsControlProviderIo>(activeConstraintsCommandReader_, activeConstraintsCommandAckReportReader_, activeConstraintsCommandStatusReader_);
  }

  void SetUp() override {
    // Code to run before running each TEST_F()
    factory_ = std::make_shared<arlcore::umaa::conditional::ConditionalFactory>(conditionalIo_);
  }

  void TearDown() override {
    factory_.reset();
    activeConstraintsCommandReader_->clear();
    activeConstraintsCommandAckReportReader_->clear();
    activeConstraintsCommandStatusReader_->clear();
    reportReader_->clear();
    conditionalSetReader_->clear();
    constraintViolatedConditionalReader_->clear();
    depthConditionalReader_->clear();
    depthRateConditionalReader_->clear();
    emitterPresetConditionalReader_->clear();
    expConditionalReader_->clear();
    headingSectorConditionalReader_->clear();
    logicalANDConditionalReader_->clear();
    logicalNOTConditionalReader_->clear();
    logicalORConditionalReader_->clear();
    missionStateConditionalReader_->clear();
    objectiveStateConditionalReader_->clear();
    pitchRateConditionalReader_->clear();
    relativeSpeedConditionalReader_->clear();
    rollRateConditionalReader_->clear();
    speedConditionalReader_->clear();
    taskStateConditionalReader_->clear();
    timeConditionalReader_->clear();
    waterZoneConditionalReader_->clear();
    yawRateConditionalReader_->clear();
  }

  std::shared_ptr<arlcore::umaa::conditional::ConditionalFactory> factory_;

  static std::shared_ptr<arlcore::umaa::ConditionalFactoryIo> conditionalIo_;
  static std::shared_ptr<ActiveConstraintsControlProviderIo> constraintsIo_;
  static std::shared_ptr<LocalReaderSender<GlobalPoseReportType>> globalPoseReader_;
  static std::shared_ptr<LocalReaderSender<SpeedReportType>> speedStatusReader_;
  static std::shared_ptr<LocalReaderSender<ActiveConstraintsCommandType>> activeConstraintsCommandReader_;
  static std::shared_ptr<LocalReaderSender<ActiveConstraintsCommandAckReportType>> activeConstraintsCommandAckReportReader_;
  static std::shared_ptr<LocalReaderSender<ActiveConstraintsCommandStatusType>> activeConstraintsCommandStatusReader_;
  static std::shared_ptr<LocalReaderSender<VelocityReportType>> velocityReportReader_;
  static std::shared_ptr<LocalReaderSender<ConditionalReportType>> reportReader_;
  static std::shared_ptr<LocalReaderSender<ConditionalReportTypeConditionalsSetElement>> conditionalSetReader_;
  static std::shared_ptr<LocalReaderSender<ConstraintViolatedConditionalType>> constraintViolatedConditionalReader_;
  static std::shared_ptr<LocalReaderSender<DepthConditionalType>> depthConditionalReader_;
  static std::shared_ptr<LocalReaderSender<DepthRateConditionalType>> depthRateConditionalReader_;
  static std::shared_ptr<LocalReaderSender<EmitterPresetConditionalType>> emitterPresetConditionalReader_;
  static std::shared_ptr<LocalReaderSender<ExpConditionalType>> expConditionalReader_;
  static std::shared_ptr<LocalReaderSender<HeadingSectorConditionalType>> headingSectorConditionalReader_;
  static std::shared_ptr<LocalReaderSender<LogicalANDConditionalType>> logicalANDConditionalReader_;
  static std::shared_ptr<LocalReaderSender<LogicalNOTConditionalType>> logicalNOTConditionalReader_;
  static std::shared_ptr<LocalReaderSender<LogicalORConditionalType>> logicalORConditionalReader_;
  static std::shared_ptr<LocalReaderSender<MissionStateConditionalType>> missionStateConditionalReader_;
  static std::shared_ptr<LocalReaderSender<ObjectiveStateConditionalType>> objectiveStateConditionalReader_;
  static std::shared_ptr<LocalReaderSender<PitchRateConditionalType>> pitchRateConditionalReader_;
  static std::shared_ptr<LocalReaderSender<RelativeSpeedConditionalType>> relativeSpeedConditionalReader_;
  static std::shared_ptr<LocalReaderSender<RollRateConditionalType>> rollRateConditionalReader_;
  static std::shared_ptr<LocalReaderSender<SpeedConditionalType>> speedConditionalReader_;
  static std::shared_ptr<LocalReaderSender<TaskStateConditionalType>> taskStateConditionalReader_;
  static std::shared_ptr<LocalReaderSender<TimeConditionalType>> timeConditionalReader_;
  static std::shared_ptr<LocalReaderSender<WaterZoneConditionalType>> waterZoneConditionalReader_;
  static std::shared_ptr<LocalReaderSender<YawRateConditionalType>> yawRateConditionalReader_;
};

std::shared_ptr<arlcore::umaa::ConditionalFactoryIo> ActiveConstraintsControlProviderTest::conditionalIo_;
std::shared_ptr<ActiveConstraintsControlProviderIo> ActiveConstraintsControlProviderTest::constraintsIo_;
std::shared_ptr<LocalReaderSender<GlobalPoseReportType>> ActiveConstraintsControlProviderTest::globalPoseReader_;
std::shared_ptr<LocalReaderSender<SpeedReportType>> ActiveConstraintsControlProviderTest::speedStatusReader_;
std::shared_ptr<LocalReaderSender<ActiveConstraintsCommandType>> ActiveConstraintsControlProviderTest::activeConstraintsCommandReader_;
std::shared_ptr<LocalReaderSender<ActiveConstraintsCommandAckReportType>> ActiveConstraintsControlProviderTest::activeConstraintsCommandAckReportReader_;
std::shared_ptr<LocalReaderSender<ActiveConstraintsCommandStatusType>> ActiveConstraintsControlProviderTest::activeConstraintsCommandStatusReader_;
std::shared_ptr<LocalReaderSender<VelocityReportType>> ActiveConstraintsControlProviderTest::velocityReportReader_;
std::shared_ptr<LocalReaderSender<ConditionalReportType>> ActiveConstraintsControlProviderTest::reportReader_;
std::shared_ptr<LocalReaderSender<ConditionalReportTypeConditionalsSetElement>> ActiveConstraintsControlProviderTest::conditionalSetReader_;
std::shared_ptr<LocalReaderSender<ConstraintViolatedConditionalType>> ActiveConstraintsControlProviderTest::constraintViolatedConditionalReader_;
std::shared_ptr<LocalReaderSender<DepthConditionalType>> ActiveConstraintsControlProviderTest::depthConditionalReader_;
std::shared_ptr<LocalReaderSender<DepthRateConditionalType>> ActiveConstraintsControlProviderTest::depthRateConditionalReader_;
std::shared_ptr<LocalReaderSender<EmitterPresetConditionalType>> ActiveConstraintsControlProviderTest::emitterPresetConditionalReader_;
std::shared_ptr<LocalReaderSender<ExpConditionalType>> ActiveConstraintsControlProviderTest::expConditionalReader_;
std::shared_ptr<LocalReaderSender<HeadingSectorConditionalType>> ActiveConstraintsControlProviderTest::headingSectorConditionalReader_;
std::shared_ptr<LocalReaderSender<LogicalANDConditionalType>> ActiveConstraintsControlProviderTest::logicalANDConditionalReader_;
std::shared_ptr<LocalReaderSender<LogicalNOTConditionalType>> ActiveConstraintsControlProviderTest::logicalNOTConditionalReader_;
std::shared_ptr<LocalReaderSender<LogicalORConditionalType>> ActiveConstraintsControlProviderTest::logicalORConditionalReader_;
std::shared_ptr<LocalReaderSender<MissionStateConditionalType>> ActiveConstraintsControlProviderTest::missionStateConditionalReader_;
std::shared_ptr<LocalReaderSender<ObjectiveStateConditionalType>> ActiveConstraintsControlProviderTest::objectiveStateConditionalReader_;
std::shared_ptr<LocalReaderSender<PitchRateConditionalType>> ActiveConstraintsControlProviderTest::pitchRateConditionalReader_;
std::shared_ptr<LocalReaderSender<RelativeSpeedConditionalType>> ActiveConstraintsControlProviderTest::relativeSpeedConditionalReader_;
std::shared_ptr<LocalReaderSender<RollRateConditionalType>> ActiveConstraintsControlProviderTest::rollRateConditionalReader_;
std::shared_ptr<LocalReaderSender<SpeedConditionalType>> ActiveConstraintsControlProviderTest::speedConditionalReader_;
std::shared_ptr<LocalReaderSender<TaskStateConditionalType>> ActiveConstraintsControlProviderTest::taskStateConditionalReader_;
std::shared_ptr<LocalReaderSender<TimeConditionalType>> ActiveConstraintsControlProviderTest::timeConditionalReader_;
std::shared_ptr<LocalReaderSender<WaterZoneConditionalType>> ActiveConstraintsControlProviderTest::waterZoneConditionalReader_;
std::shared_ptr<LocalReaderSender<YawRateConditionalType>> ActiveConstraintsControlProviderTest::yawRateConditionalReader_;

TEST_F(ActiveConstraintsControlProviderTest, getConstraintConditionals) {
  auto consumer = std::make_shared<arlcore::umaa::conditional::ConditionalReportConsumer>(reportReader_, conditionalSetReader_, factory_);
  NumericGuid source = arlcore::UuidFactory::getInstance().generateGuid();
  auto constraintProvider = std::make_shared<arlcore::umaa::conditional::ActiveConstraintsControlProvider>(source, constraintsIo_);
  consumer->registerObserver(constraintProvider);
  arlcore::NumericGuid id0 = arlcore::UuidFactory::getInstance().parseGuidFromString("00000000-0000-0000-0000-100000000000");
  arlcore::NumericGuid id1 = arlcore::UuidFactory::getInstance().parseGuidFromString("00000000-0000-0000-0000-200000000000");
  arlcore::umaa::LargeSetWriter<ConditionalType, ConditionalReportTypeConditionalsSetElement> writer(conditionalSetReader_);
  ConditionalType c0, c1;
  c0.name("C0");
  c1.name("C1");
  c0.conditionalID(id0.getGuid());
  c1.conditionalID(id1.getGuid());
  c0.specializationID(id0.getGuid());
  c1.specializationID(id1.getGuid());
  c0.specializationTimestamp(UMAA::Common::Measurement::DateTime(0, 0));
  c1.specializationTimestamp(UMAA::Common::Measurement::DateTime(0, 0));
  c0.specializationTopic(UMAA::MM::Conditional::DepthConditionalTypeTopic);
  c1.specializationTopic(UMAA::MM::Conditional::PitchRateConditionalTypeTopic);
  writer.insert(c0);
  writer.insert(c1);

  UMAA::MM::Conditional::DepthConditionalType d0(ConditionalOperatorEnumType::GREATER_THAN, 10, c0.specializationTimestamp(), c0.specializationID());
  UMAA::MM::Conditional::PitchRateConditionalType p0(ConditionalOperatorEnumType::LESS_THAN, 10, c1.specializationTimestamp(), c1.specializationID());

  depthConditionalReader_->send(d0);
  pitchRateConditionalReader_->send(p0);

  conditionalIo_->DepthCache.update();
  conditionalIo_->PitchRateCache.update();

  ConditionalReportType report;
  auto metadata = writer.getMetadata();
  report.conditionalsSetMetadata(metadata);
  report.source().id(arlcore::NIL_GUID.getGuid());
  report.timeStamp(metadata.updateElementTimestamp().value());
  
  EXPECT_FALSE(consumer->getReport().has_value());
  EXPECT_FALSE(consumer->getConditionals().has_value());
  reportReader_->send(report);
  consumer->cycle();

  ASSERT_TRUE(consumer->getReport().has_value());
  EXPECT_EQ(report, consumer->getReport().value());

  ActiveConstraintsCommandType command;
  command.constraintConditionalIDs().push_back(id0);
  
  EXPECT_EQ(activeConstraintsCommandReader_->send(command), SendStatus::SUCCESS);
  constraintProvider->cycle();
  // EXPECT_EQ(constraintProvider->getCommandStatus().value(), CommandStatusEnumType::ISSUED);
  // constraintProvider->cycle();
  // EXPECT_EQ(constraintProvider->getCommandStatus().value(), CommandStatusEnumType::COMMANDED);
  // constraintProvider->cycle();
  // EXPECT_EQ(constraintProvider->getCommandStatus().value(), CommandStatusEnumType::EXECUTING);
  // constraintProvider->cycle();
  // EXPECT_EQ(constraintProvider->getCommandStatus().value(), CommandStatusEnumType::COMPLETED);

  ActiveConstraintsCommandAckReportType ack;
  EXPECT_EQ(activeConstraintsCommandAckReportReader_->read(&ack), ReadStatus::SUCCESS);
  EXPECT_EQ(ack.source().id(), source.getGuid());
  EXPECT_EQ(ack.command(), command);

  ActiveConstraintsCommandStatusType status;
  EXPECT_EQ(activeConstraintsCommandStatusReader_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.source().id(), source.getGuid());
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::ISSUED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  EXPECT_EQ(activeConstraintsCommandStatusReader_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMMANDED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  EXPECT_EQ(activeConstraintsCommandStatusReader_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::EXECUTING);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  EXPECT_EQ(activeConstraintsCommandStatusReader_->read(&status), ReadStatus::SUCCESS);
  EXPECT_EQ(status.commandStatus(), CommandStatusEnumType::COMPLETED);
  EXPECT_EQ(status.commandStatusReason(), CommandStatusReasonEnumType::SUCCEEDED);
  
  auto constraints = constraintProvider->getConstraintConditionals();
  ASSERT_TRUE(constraints.has_value());
  EXPECT_EQ(constraints->size(), 1);
  EXPECT_EQ(constraints->at(0)->getConditionalId(), id0);
  EXPECT_EQ(constraints->at(0)->getSpecializationId(), id0);
}

//! Shared setup for the standing-session tests: publishes a conditional report holding two depth/pitch-rate
//! conditionals (id0/id1) and cycles the given consumer so both are resolvable.
class StandingActiveConstraintsTest : public ActiveConstraintsControlProviderTest {
 protected:
  void publishConditionalSet(std::shared_ptr<arlcore::umaa::conditional::ConditionalReportConsumer> consumer) {
    id0_ = arlcore::UuidFactory::getInstance().generateGuid();
    id1_ = arlcore::UuidFactory::getInstance().generateGuid();
    c0_.name("C0");
    c1_.name("C1");
    c0_.conditionalID(id0_.getGuid());
    c1_.conditionalID(id1_.getGuid());
    c0_.specializationID(id0_.getGuid());
    c1_.specializationID(id1_.getGuid());
    c0_.specializationTimestamp(UMAA::Common::Measurement::DateTime(0, 0));
    c1_.specializationTimestamp(UMAA::Common::Measurement::DateTime(0, 0));
    c0_.specializationTopic(UMAA::MM::Conditional::DepthConditionalTypeTopic);
    c1_.specializationTopic(UMAA::MM::Conditional::PitchRateConditionalTypeTopic);
    setWriter_ = std::make_unique<arlcore::umaa::LargeSetWriter<ConditionalType,
      ConditionalReportTypeConditionalsSetElement>>(conditionalSetReader_);
    setWriter_->insert(c0_);
    setWriter_->insert(c1_);

    UMAA::MM::Conditional::DepthConditionalType d0(ConditionalOperatorEnumType::GREATER_THAN, 10,
      c0_.specializationTimestamp(), c0_.specializationID());
    UMAA::MM::Conditional::PitchRateConditionalType p0(ConditionalOperatorEnumType::LESS_THAN, 10,
      c1_.specializationTimestamp(), c1_.specializationID());
    depthConditionalReader_->send(d0);
    pitchRateConditionalReader_->send(p0);
    conditionalIo_->DepthCache.update();
    conditionalIo_->PitchRateCache.update();

    sendReport(consumer);
  }

  void sendReport(std::shared_ptr<arlcore::umaa::conditional::ConditionalReportConsumer> consumer) {
    ConditionalReportType report;
    auto metadata = setWriter_->getMetadata();
    report.conditionalsSetMetadata(metadata);
    report.source().id(arlcore::NIL_GUID.getGuid());
    // A removal resets the metadata's updateElementTimestamp, so stamp the report independently
    report.timeStamp(arlcore::umaa::getTimestamp());
    reportReader_->send(report);
    consumer->cycle();
  }

  ActiveConstraintsCommandType makeCommand(const std::vector<arlcore::NumericGuid>& ids) {
    ActiveConstraintsCommandType command;
    for (const auto& id : ids) {
      command.constraintConditionalIDs().push_back(id.getGuid());
    }
    command.sessionID(arlcore::UuidFactory::getInstance().generateGuid().getGuid());
    return command;
  }

  //! Read command statuses, skipping DISPOSED instance samples, until a live status is found
  std::optional<ActiveConstraintsCommandStatusType> nextLiveStatus() {
    ActiveConstraintsCommandStatusType status;
    ReadStatus read = activeConstraintsCommandStatusReader_->read(&status);
    while (read == ReadStatus::DISPOSED) {
      read = activeConstraintsCommandStatusReader_->read(&status);
    }
    if (read != ReadStatus::SUCCESS) {
      return std::nullopt;
    }
    return status;
  }

  arlcore::NumericGuid id0_;
  arlcore::NumericGuid id1_;
  ConditionalType c0_;
  ConditionalType c1_;
  std::unique_ptr<arlcore::umaa::LargeSetWriter<ConditionalType,
    ConditionalReportTypeConditionalsSetElement>> setWriter_;
};

TEST_F(StandingActiveConstraintsTest, standingSessionStaysExecutingAndSurvivesDispose) {
  auto consumer = std::make_shared<arlcore::umaa::conditional::ConditionalReportConsumer>(reportReader_,
    conditionalSetReader_, factory_);
  NumericGuid source = arlcore::UuidFactory::getInstance().generateGuid();
  auto provider = std::make_shared<arlcore::umaa::conditional::ActiveConstraintsControlProvider>(source,
    constraintsIo_, true);
  consumer->registerObserver(provider);
  publishConditionalSet(consumer);

  ActiveConstraintsCommandType command = makeCommand({id0_});
  EXPECT_EQ(activeConstraintsCommandReader_->send(command), SendStatus::SUCCESS);
  EXPECT_TRUE(provider->cycle());

  // The session latches EXECUTING instead of completing
  ASSERT_TRUE(provider->getCommandStatus().has_value());
  EXPECT_EQ(provider->getCommandStatus().value(), CommandStatusEnumType::EXECUTING);
  auto constraints = provider->getConstraintConditionals();
  ASSERT_TRUE(constraints.has_value());
  ASSERT_EQ(constraints->size(), 1);
  EXPECT_EQ(constraints->at(0)->getConditionalId(), id0_);

  // ... and stays there over subsequent cycles
  EXPECT_TRUE(provider->cycle());
  EXPECT_EQ(provider->getCommandStatus().value(), CommandStatusEnumType::EXECUTING);

  // A commander restart autodisposes its command instance; the standing session must ignore it
  EXPECT_EQ(activeConstraintsCommandReader_->dispose(command), SendStatus::SUCCESS);
  EXPECT_TRUE(provider->cycle());
  ASSERT_TRUE(provider->getCommandStatus().has_value());
  EXPECT_EQ(provider->getCommandStatus().value(), CommandStatusEnumType::EXECUTING);
  constraints = provider->getConstraintConditionals();
  ASSERT_TRUE(constraints.has_value());
  EXPECT_EQ(constraints->size(), 1);

  // No CANCELED status was ever published
  std::optional<ActiveConstraintsCommandStatusType> status = nextLiveStatus();
  while (status.has_value()) {
    EXPECT_NE(status->commandStatus(), CommandStatusEnumType::CANCELED);
    status = nextLiveStatus();
  }
}

TEST_F(StandingActiveConstraintsTest, standingSessionSupersededByNewCommand) {
  auto consumer = std::make_shared<arlcore::umaa::conditional::ConditionalReportConsumer>(reportReader_,
    conditionalSetReader_, factory_);
  NumericGuid source = arlcore::UuidFactory::getInstance().generateGuid();
  auto provider = std::make_shared<arlcore::umaa::conditional::ActiveConstraintsControlProvider>(source,
    constraintsIo_, true);
  consumer->registerObserver(provider);
  publishConditionalSet(consumer);

  ActiveConstraintsCommandType first = makeCommand({id0_});
  EXPECT_EQ(activeConstraintsCommandReader_->send(first), SendStatus::SUCCESS);
  EXPECT_TRUE(provider->cycle());
  EXPECT_EQ(provider->getCommandStatus().value(), CommandStatusEnumType::EXECUTING);

  // Drain the first session's statuses so only supersede traffic remains
  while (nextLiveStatus().has_value()) {}

  ActiveConstraintsCommandType second = makeCommand({id1_});
  EXPECT_EQ(activeConstraintsCommandReader_->send(second), SendStatus::SUCCESS);
  EXPECT_TRUE(provider->cycle());

  // The new command supersedes: old session CANCELED, new session standing in EXECUTING
  std::optional<ActiveConstraintsCommandStatusType> status = nextLiveStatus();
  ASSERT_TRUE(status.has_value());
  EXPECT_EQ(status->commandStatus(), CommandStatusEnumType::CANCELED);
  EXPECT_EQ(NumericGuid(status->sessionID()), NumericGuid(first.sessionID()));

  ASSERT_TRUE(provider->getCommandStatus().has_value());
  EXPECT_EQ(provider->getCommandStatus().value(), CommandStatusEnumType::EXECUTING);
  EXPECT_EQ(NumericGuid(provider->getActiveCommand()->sessionID()), NumericGuid(second.sessionID()));

  auto constraints = provider->getConstraintConditionals();
  ASSERT_TRUE(constraints.has_value());
  ASSERT_EQ(constraints->size(), 1);
  EXPECT_EQ(constraints->at(0)->getConditionalId(), id1_);

  // Exactly one live ack remains, mirroring the applied command
  ActiveConstraintsCommandAckReportType ack;
  ReadStatus ackRead = activeConstraintsCommandAckReportReader_->read(&ack);
  std::optional<ActiveConstraintsCommandAckReportType> lastLiveAck;
  int liveAcks = 0;
  while (ackRead != ReadStatus::NO_DATA) {
    if (ackRead == ReadStatus::SUCCESS) {
      lastLiveAck = ack;
      ++liveAcks;
    } else if (ackRead == ReadStatus::DISPOSED && lastLiveAck.has_value() &&
        NumericGuid(ack.sessionID()) == NumericGuid(lastLiveAck->sessionID())) {
      lastLiveAck.reset();
      --liveAcks;
    }
    ackRead = activeConstraintsCommandAckReportReader_->read(&ack);
  }
  ASSERT_TRUE(lastLiveAck.has_value());
  EXPECT_EQ(NumericGuid(lastLiveAck->sessionID()), NumericGuid(second.sessionID()));
  EXPECT_EQ(lastLiveAck->command(), second);
}

TEST_F(StandingActiveConstraintsTest, standingSessionDeactivatesDeletedConditionalAndReactivatesOnReAdd) {
  auto consumer = std::make_shared<arlcore::umaa::conditional::ConditionalReportConsumer>(reportReader_,
    conditionalSetReader_, factory_);
  NumericGuid source = arlcore::UuidFactory::getInstance().generateGuid();
  auto provider = std::make_shared<arlcore::umaa::conditional::ActiveConstraintsControlProvider>(source,
    constraintsIo_, true);
  consumer->registerObserver(provider);
  publishConditionalSet(consumer);

  ActiveConstraintsCommandType command = makeCommand({id0_});
  EXPECT_EQ(activeConstraintsCommandReader_->send(command), SendStatus::SUCCESS);
  EXPECT_TRUE(provider->cycle());
  ASSERT_EQ(provider->getConstraintConditionals()->size(), 1);

  // The active conditional is deleted from the report: it deactivates, but the session stays EXECUTING
  setWriter_->remove(c0_);
  sendReport(consumer);
  EXPECT_TRUE(provider->cycle());
  EXPECT_EQ(provider->getCommandStatus().value(), CommandStatusEnumType::EXECUTING);
  ASSERT_TRUE(provider->getConstraintConditionals().has_value());
  EXPECT_TRUE(provider->getConstraintConditionals()->empty());

  // Re-adding the conditional under the same ID re-activates it (commander edit flow)
  setWriter_->insert(c0_);
  sendReport(consumer);
  EXPECT_TRUE(provider->cycle());
  ASSERT_TRUE(provider->getConstraintConditionals().has_value());
  ASSERT_EQ(provider->getConstraintConditionals()->size(), 1);
  EXPECT_EQ(provider->getConstraintConditionals()->at(0)->getConditionalId(), id0_);

  // Deleting EVERY conditional (empty set) must still notify: the active set drains
  setWriter_->remove(c0_);
  setWriter_->remove(c1_);
  sendReport(consumer);
  EXPECT_TRUE(provider->cycle());
  EXPECT_EQ(provider->getCommandStatus().value(), CommandStatusEnumType::EXECUTING);
  ASSERT_TRUE(provider->getConstraintConditionals().has_value());
  EXPECT_TRUE(provider->getConstraintConditionals()->empty());
}
